use crate::protocol::{
    base::types::*,
    structures::{
        Location,
        PartialResultParams,
        Position,
        Range,
        TextDocumentIdentifier,
        WorkDoneProgressParams,
        make_file_uri,
    },
};
use serde::{
    Deserialize,
    Serialize
};
use serde_repr::{
    Deserialize_repr,
    Serialize_repr,
};

/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#documentSymbolParams
#[derive(Serialize)]
pub struct DocumentSymbolParams {
    #[serde(flatten)]
    pub work_done_progress: WorkDoneProgressParams,

    #[serde(flatten)]
    pub partial_result_params: PartialResultParams,

	// The text document.
    #[serde(rename = "textDocument")]
	pub text_document: TextDocumentIdentifier,
}

impl DocumentSymbolParams {
    pub fn new(uri: String) -> Self {
        Self {
            work_done_progress: WorkDoneProgressParams::default(),
            partial_result_params: PartialResultParams::default(),
            text_document: TextDocumentIdentifier {
                uri: make_file_uri(&uri),
            },
        }
    }
}

/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#symbolKind
///
/// A symbol kind.
#[derive(Clone, Copy, PartialEq, Eq, Debug, Serialize_repr, Deserialize_repr)]
#[repr(u8)]
pub enum SymbolKind {
    File = 1,
	Module = 2,
	Namespace = 3,
	Package = 4,
	Class = 5,
	Method = 6,
	Property = 7,
	Field = 8,
	Constructor = 9,
	Enum = 10,
	Interface = 11,
	Function = 12,
	Variable = 13,
	Constant = 14,
	String = 15,
	Number = 16,
	Boolean = 17,
	Array = 18,
	Object = 19,
	Key = 20,
	Null = 21,
	EnumMember = 22,
	Struct = 23,
	Event = 24,
	Operator = 25,
	TypeParameter = 26,
}

/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#symbolTag
///
/// Symbol tags are extra annotations that tweak the rendering of a symbol.
///
/// @since 3.16
#[derive(Serialize, Deserialize)]
pub enum SymbolTag {
	// Render a symbol as obsolete, usually using a strike-out.
	Deprecated = 1,
}

/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#documentSymbol
///
/// Represents programming constructs like variables, classes, interfaces etc.
/// that appear in a document. Document symbols can be hierarchical and they
/// have two ranges: one that encloses its definition and one that points to its
/// most interesting range, e.g. the range of an identifier.
#[derive(Deserialize)]
pub struct DocumentSymbol {
	// The name of this symbol. Will be displayed in the user interface and
	// therefore must not be an empty string or a string only consisting of
	// white spaces.
	pub name: String,

	// More detail for this symbol, e.g the signature of a function.
	pub detail: Option<String>,

	// The kind of this symbol.
	pub kind: SymbolKind,

	// Tags for this document symbol.
	//
	// @since 3.16.0
	pub tags: Option<Vec<SymbolTag>>,

	// Indicates if this symbol is deprecated.
	//
	// @deprecated Use tags instead
	pub deprecated: Option<Boolean>,

	// The range enclosing this symbol not including leading/trailing whitespace
	// but everything else like comments. This information is typically used to
	// determine if the clients cursor is inside the symbol to reveal it  in the
	// UI.
	pub range: Range,

	// The range that should be selected and revealed when this symbol is being
	// picked, e.g. the name of a function. Must be contained by the `range`.
    #[serde(rename = "selectionRange")]
	pub selection_range: Range,

	// Children of this symbol, e.g. properties of a class.
	pub children: Option<Vec<DocumentSymbol>>,
}

/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#symbolInformation
///
/// Represents information about programming constructs like variables, classes,
/// interfaces etc.
///
/// @deprecated use DocumentSymbol or WorkspaceSymbol instead.
#[derive(Deserialize)]
pub struct SymbolInformation {
	// The name of this symbol.
	pub name: String,

	// The kind of this symbol.
	pub kind: SymbolKind,

	// Tags for this symbol.
	//
	// @since 3.16.0
	pub tags: Option<Vec<SymbolTag>>,

	// Indicates if this symbol is deprecated.
	//
	// @deprecated Use tags instead
	pub deprecated: Option<Boolean>,

	// The location of this symbol. The location's range is used by a tool
	// to reveal the location in the editor. If the symbol is selected in the
	// tool the range's start information is used to position the cursor. So
	// the range usually spans more then the actual symbol's name and does
	// normally include things like visibility modifiers.
	//
	// The range doesn't have to denote a node range in the sense of an abstract
	// syntax tree. It can therefore not be used to re-construct a hierarchy of
	// the symbols.
	pub location: Location,

	// The name of the symbol containing this symbol. This information is for
	// user interface purposes (e.g. to render a qualifier in the user interface
	// if necessary). It can't be used to re-infer a hierarchy for the document
	// symbols.
    #[serde(rename = "containerName")]
	pub container_name: Option<String>,
}
