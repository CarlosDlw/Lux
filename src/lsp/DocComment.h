#pragma once

#include <string>
#include <vector>

// ═══════════════════════════════════════════════════════════════════════
//  Doc-comment data model
// ═══════════════════════════════════════════════════════════════════════

// Kind of a doc-tag — determines parsing semantics and hover formatting.
enum class DocTagKind {
    // name + description
    Param,       // @param name description
    Property,    // @property name description
    Field,       // @field name description

    // text / description
    Return,      // @return description
    Returns,     // @returns description
    Brief,       // @brief description
    Deprecated,  // @deprecated [reason]
    Version,     // @version semver
    Author,      // @author name
    See,         // @see reference
    Since,       // @since version
    Throws,      // @throws description
    Todo,        // @todo description

    // multi-line block (content until next @tag)
    Example,     // @example
    Remarks,     // @remarks
    Note,        // @note
    Warning,     // @warning

    // flags (no arguments)
    Private,     // @private
    Public,      // @public
    Protected,   // @protected
    Internal,    // @internal
    Struct,      // @struct
    Namespace,   // @namespace
};

struct DocTag {
    DocTagKind  kind;
    std::string name;   // for Param/Property/Field
    std::string text;   // description / content
};

struct DocComment {
    std::string          summary;    // text before first @tag
    std::vector<DocTag>  tags;
    size_t               startLine;  // 0-based
    size_t               endLine;    // 0-based
};

// ═══════════════════════════════════════════════════════════════════════
//  Parsing
// ═══════════════════════════════════════════════════════════════════════

// Extract all /** ... */ doc-comments from raw source.
std::vector<DocComment> parseDocComments(const std::string& source);

// Find the doc-comment whose endLine is immediately before `declLine` (0-based).
// Returns nullptr if no matching doc-comment exists.
const DocComment* findDocForLine(const std::vector<DocComment>& docs,
                                 size_t declLine);

// ═══════════════════════════════════════════════════════════════════════
//  Formatting
// ═══════════════════════════════════════════════════════════════════════

// Format a DocComment as Markdown for LSP hover.
std::string formatDocComment(const DocComment& doc);

// Append a doc-comment (if found) to existing hover markdown.
// `declLine` is the 0-based line of the declaration.
std::string appendDocToHover(const std::string& hoverMd,
                             const std::vector<DocComment>& docs,
                             size_t declLine);

// ═══════════════════════════════════════════════════════════════════════
//  Completion helpers
// ═══════════════════════════════════════════════════════════════════════

// Check whether the cursor (0-based line:col) is inside a /** ... */ block.
bool isInsideDocComment(const std::string& source, size_t line, size_t col);
