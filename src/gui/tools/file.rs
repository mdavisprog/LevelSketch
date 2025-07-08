use bevy::prelude::*;
use kea::prelude::*;
use super::tools::Tools;

#[derive(Component)]
#[require(Tools)]
pub struct FileTools {
    _private: (),
}

impl FileTools {
    pub fn bundle() -> impl Bundle {(
        Self {
            _private: (),
        },
        children![(
            KeaButton::label_bundle( "Quit", |
                _: Trigger<KeaButtonClick>,
                mut events: EventWriter<AppExit>,
                | {
                    events.write(AppExit::Success);
                },
            ),
        )]
    )}
}
