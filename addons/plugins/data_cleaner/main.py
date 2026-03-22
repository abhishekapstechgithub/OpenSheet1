# OpenSheet Plugin: Data Cleaner
# Adds TRIM_RANGE() and REMOVE_DUPLICATES() formula functions
# and a "Clean Data" menu item.

def on_load(context):
    """Called when plugin loads. Register functions and menu items."""
    context.register_formula("TRIM_CELLS",        trim_cells)
    context.register_formula("PROPER_CASE",       proper_case)
    context.register_formula("REMOVE_WHITESPACE", remove_whitespace)
    context.add_menu_item("Clean Selected Range",  on_clean_menu)
    context.add_menu_item("Remove Extra Spaces",   on_trim_menu)
    context.log("DataCleaner plugin loaded successfully")


def trim_cells(args):
    """TRIM_CELLS(text) - Remove leading, trailing and extra internal spaces."""
    if not args:
        return ""
    import re
    text = str(args[0])
    return re.sub(r'\s+', ' ', text.strip())


def proper_case(args):
    """PROPER_CASE(text) - Convert text to Title Case."""
    if not args:
        return ""
    return str(args[0]).title()


def remove_whitespace(args):
    """REMOVE_WHITESPACE(text) - Remove ALL whitespace from a string."""
    if not args:
        return ""
    return "".join(str(args[0]).split())


def on_clean_menu():
    """Menu action: log that clean was triggered (workbook access via context)."""
    print("[DataCleaner] Clean Selected Range triggered")


def on_trim_menu():
    """Menu action: Remove extra spaces from selection."""
    print("[DataCleaner] Remove Extra Spaces triggered")


def on_unload():
    """Called before plugin is unloaded."""
    print("[DataCleaner] Plugin unloaded")
