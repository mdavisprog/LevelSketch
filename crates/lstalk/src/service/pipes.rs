use async_process::ChildStdin;
use futures_lite::{
    io::BufReader,
    prelude::*,
};
use std::task::{
    Context,
    Poll,
    Waker,
};

pub struct ReadPipe<'a, T: AsyncRead + Unpin> {
    reader: BufReader<T>,
    context: Context<'a>,
}

impl<'a, T: AsyncRead + Unpin> ReadPipe<'a, T> {
    pub fn new(stream: T) -> Self {
        Self {
            reader: BufReader::new(stream),
            context: Context::from_waker(Waker::noop()),
        }
    }

    pub fn read(&mut self) -> Option<String> {
        const PACKET_SIZE: usize = 1024;

        let mut result = String::new();
        let mut packet: [u8; PACKET_SIZE] = [0; PACKET_SIZE];

        loop {
            match self.reader.read(&mut packet).poll(&mut self.context) {
                Poll::Pending => {
                    break;
                },
                Poll::Ready(num_bytes) => {
                    let num_bytes = match num_bytes {
                        Ok(num) => num,
                        Err(error) => {
                            println!("Failed to poll read operation: {error:?}");
                            break;
                        }
                    };

                    let extend = match String::from_utf8(packet[0..num_bytes].into()) {
                        Ok(result) => result,
                        Err(error) => {
                            println!("Failed to convert bytes to utf-8: {error:?}");
                            break;
                        },
                    };

                    result += &extend;

                    if num_bytes < PACKET_SIZE {
                        break;
                    }
                },
            }
        }

        if result.is_empty() {
            None
        } else {
            Some(result)
        }
    }
}

pub struct WritePipe<'a> {
    stdin: ChildStdin,
    context: Context<'a>,
    stack: Vec<String>,
    written: usize,
}

impl<'a> WritePipe<'a> {
    pub fn new(stdin: ChildStdin) -> Self {
        Self {
            stdin,
            context: Context::from_waker(Waker::noop()),
            stack: Vec::new(),
            written: 0,
        }
    }

    pub fn write(&mut self, buffer: String) {
        self.stack.push(buffer);
    }

    pub fn poll(&mut self) {
        let Some(top) = self.stack.last() else {
            return;
        };

        match self.stdin.write(top.as_bytes()).poll(&mut self.context) {
            Poll::Pending => {},
            Poll::Ready(result) => {
                if let Ok(written) = result {
                    self.written += written;

                    if self.written >= top.len() {
                        self.stack.pop();
                        self.written = 0;
                    }
                }
            },
        }
    }
}
