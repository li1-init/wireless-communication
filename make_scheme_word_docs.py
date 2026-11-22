from __future__ import annotations

import re
import sys
from pathlib import Path

from docx import Document
from docx.enum.section import WD_SECTION
from docx.enum.table import WD_TABLE_ALIGNMENT, WD_CELL_VERTICAL_ALIGNMENT
from docx.enum.text import WD_ALIGN_PARAGRAPH, WD_LINE_SPACING
from docx.enum.style import WD_STYLE_TYPE
from docx.oxml import OxmlElement
from docx.oxml.ns import qn
from docx.shared import Cm, Pt, RGBColor


FONT = "Microsoft YaHei"
BODY_PT = 14
BLACK = RGBColor(0, 0, 0)


def set_font(run, size=BODY_PT, bold=None):
    run.font.name = FONT
    run.font.size = Pt(size)
    run.font.color.rgb = BLACK
    if bold is not None:
        run.bold = bold
    rpr = run._element.get_or_add_rPr()
    rfonts = rpr.rFonts
    if rfonts is None:
        rfonts = OxmlElement("w:rFonts")
        rpr.insert(0, rfonts)
    for attr in ("ascii", "hAnsi", "eastAsia", "cs"):
        rfonts.set(qn(f"w:{attr}"), FONT)


def configure_style(style, size, bold=False, outline=None):
    style.font.name = FONT
    style.font.size = Pt(size)
    style.font.bold = bold
    style.font.color.rgb = BLACK
    rpr = style.element.get_or_add_rPr()
    rfonts = rpr.rFonts
    if rfonts is None:
        rfonts = OxmlElement("w:rFonts")
        rpr.insert(0, rfonts)
    for attr in ("ascii", "hAnsi", "eastAsia", "cs"):
        rfonts.set(qn(f"w:{attr}"), FONT)
    if outline is not None:
        ppr = style.element.get_or_add_pPr()
        level = OxmlElement("w:outlineLvl")
        level.set(qn("w:val"), str(outline))
        ppr.append(level)


def add_page_field(paragraph):
    run = paragraph.add_run()
    set_font(run, 12)
    begin = OxmlElement("w:fldChar")
    begin.set(qn("w:fldCharType"), "begin")
    instr = OxmlElement("w:instrText")
    instr.set(qn("xml:space"), "preserve")
    instr.text = " PAGE "
    separate = OxmlElement("w:fldChar")
    separate.set(qn("w:fldCharType"), "separate")
    text = OxmlElement("w:t")
    text.text = "1"
    end = OxmlElement("w:fldChar")
    end.set(qn("w:fldCharType"), "end")
    for element in (begin, instr, separate, text, end):
        run._r.append(element)


def add_toc(paragraph):
    fld = OxmlElement("w:fldSimple")
    fld.set(qn("w:instr"), r' TOC \o "1-2" \h \z \u ')
    paragraph._p.append(fld)


def add_inline(paragraph, text):
    parts = re.split(r"(`[^`]+`|\*\*[^*]+\*\*)", text)
    for part in parts:
        if not part:
            continue
        bold = part.startswith("**") and part.endswith("**")
        code = part.startswith("`") and part.endswith("`")
        value = part[2:-2] if bold else part[1:-1] if code else part
        run = paragraph.add_run(value)
        set_font(run, bold=True if bold or code else None)


def add_border_left(paragraph):
    ppr = paragraph._p.get_or_add_pPr()
    borders = OxmlElement("w:pBdr")
    left = OxmlElement("w:left")
    left.set(qn("w:val"), "single")
    left.set(qn("w:sz"), "4")
    left.set(qn("w:space"), "6")
    left.set(qn("w:color"), "000000")
    borders.append(left)
    ppr.append(borders)


def set_cell_border(cell):
    tcpr = cell._tc.get_or_add_tcPr()
    borders = tcpr.first_child_found_in("w:tcBorders")
    if borders is None:
        borders = OxmlElement("w:tcBorders")
        tcpr.append(borders)
    for edge in ("top", "left", "bottom", "right", "insideH", "insideV"):
        tag = f"w:{edge}"
        element = borders.find(qn(tag))
        if element is None:
            element = OxmlElement(tag)
            borders.append(element)
        element.set(qn("w:val"), "single")
        element.set(qn("w:sz"), "4")
        element.set(qn("w:color"), "000000")


def repeat_table_header(row):
    trpr = row._tr.get_or_add_trPr()
    header = OxmlElement("w:tblHeader")
    header.set(qn("w:val"), "true")
    trpr.append(header)


def add_table(doc, rows):
    table = doc.add_table(rows=1, cols=len(rows[0]))
    table.style = "Table Grid"
    table.alignment = WD_TABLE_ALIGNMENT.CENTER
    table.autofit = True
    repeat_table_header(table.rows[0])
    for row_index, values in enumerate(rows):
        cells = table.rows[0].cells if row_index == 0 else table.add_row().cells
        for cell, value in zip(cells, values):
            cell.vertical_alignment = WD_CELL_VERTICAL_ALIGNMENT.CENTER
            set_cell_border(cell)
            cell.text = ""
            paragraph = cell.paragraphs[0]
            paragraph.paragraph_format.line_spacing = 1.2
            paragraph.paragraph_format.space_after = Pt(0)
            add_inline(paragraph, value)
            for run in paragraph.runs:
                set_font(run, BODY_PT, bold=(row_index == 0))
    doc.add_paragraph()


def parse_table(lines):
    rows = []
    for line in lines:
        values = [part.strip() for part in line.strip().strip("|").split("|")]
        if values and all(re.fullmatch(r":?-{3,}:?", value) for value in values):
            continue
        rows.append(values)
    return rows


def add_toc_block(doc):
    paragraph = doc.add_paragraph()
    paragraph.paragraph_format.space_after = Pt(6)
    add_toc(paragraph)
    doc.add_page_break()


def convert(source, target):
    lines = source.read_text(encoding="utf-8-sig").splitlines()
    doc = Document()
    section = doc.sections[0]
    section.page_width = Cm(21)
    section.page_height = Cm(29.7)
    section.top_margin = Cm(2.1)
    section.bottom_margin = Cm(2.1)
    section.left_margin = Cm(2.1)
    section.right_margin = Cm(2.1)

    configure_style(doc.styles["Normal"], BODY_PT)
    normal = doc.styles["Normal"]
    normal.paragraph_format.line_spacing_rule = WD_LINE_SPACING.MULTIPLE
    normal.paragraph_format.line_spacing = 1.45
    normal.paragraph_format.space_after = Pt(6)
    configure_style(doc.styles["Title"], 24, True)
    configure_style(doc.styles["Heading 1"], 20, True, 0)
    configure_style(doc.styles["Heading 2"], 16, True, 1)
    for name in ("Hyperlink", "FollowedHyperlink"):
        try:
            style = doc.styles[name]
        except KeyError:
            style = doc.styles.add_style(name, WD_STYLE_TYPE.CHARACTER)
        style.font.color.rgb = BLACK
        style.font.underline = False

    footer = section.footer.paragraphs[0]
    footer.alignment = WD_ALIGN_PARAGRAPH.CENTER
    footer.paragraph_format.space_after = Pt(0)
    title_text = next(
        (line[2:].strip() for line in lines if line.startswith("# ")),
        source.stem,
    )
    add_inline(footer, f"第 ")
    add_page_field(footer)
    add_inline(footer, f" 页 | {title_text}")
    for run in footer.runs:
        set_font(run, 12)

    i = 0
    toc_inserted = False
    while i < len(lines):
        raw = lines[i].strip()
        if not raw:
            i += 1
            continue
        if raw.startswith("# "):
            paragraph = doc.add_paragraph(style="Title")
            add_inline(paragraph, raw[2:].strip())
            for run in paragraph.runs:
                set_font(run, 24, True)
            if not toc_inserted:
                add_toc_block(doc)
                toc_inserted = True
            i += 1
            continue
        if raw.startswith("## "):
            paragraph = doc.add_paragraph(style="Heading 1")
            add_inline(paragraph, raw[3:].strip())
            i += 1
            continue
        if raw.startswith("### "):
            paragraph = doc.add_paragraph(style="Heading 2")
            add_inline(paragraph, raw[4:].strip())
            i += 1
            continue
        if raw.startswith("```"):
            block = []
            i += 1
            while i < len(lines) and not lines[i].strip().startswith("```"):
                block.append(lines[i])
                i += 1
            paragraph = doc.add_paragraph()
            paragraph.paragraph_format.line_spacing = 1.2
            paragraph.paragraph_format.left_indent = Cm(0.5)
            paragraph.paragraph_format.space_after = Pt(6)
            add_border_left(paragraph)
            add_inline(paragraph, "\n".join(block))
            i += 1
            continue
        if raw.startswith("|"):
            table_lines = []
            while i < len(lines) and lines[i].strip().startswith("|"):
                table_lines.append(lines[i].strip())
                i += 1
            rows = parse_table(table_lines)
            if rows:
                add_table(doc, rows)
            continue
        if raw.startswith("- "):
            paragraph = doc.add_paragraph(style="List Bullet")
            paragraph.paragraph_format.space_after = Pt(6)
            add_inline(paragraph, raw[2:].strip())
            i += 1
            continue
        paragraph = doc.add_paragraph()
        add_inline(paragraph, raw.replace("  ", " "))
        i += 1

    settings = doc.settings.element
    update_fields = settings.find(qn("w:updateFields"))
    if update_fields is None:
        update_fields = OxmlElement("w:updateFields")
        settings.append(update_fields)
    update_fields.set(qn("w:val"), "true")
    doc.save(target)


def main():
    if len(sys.argv) != 2:
        raise SystemExit("Usage: make_scheme_word_docs.py <project-directory>")
    root = Path(sys.argv[1])
    for source in sorted(root.glob("Dishu-WH_方案*_按当前工程更新版.md")):
        target = source.with_suffix(".docx")
        convert(source, target)
        print(f"{source.name} -> {target.name}")


if __name__ == "__main__":
    main()
