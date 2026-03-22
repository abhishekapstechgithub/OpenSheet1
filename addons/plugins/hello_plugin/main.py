"""
hello_plugin/main.py
Example OpenSheet Python plugin.

The OpenSheet plugin context is exposed via the `opensheet` module
which is injected by the Python plugin loader at runtime.
"""

# import opensheet  # available at runtime

PLUGIN_NAME    = "Hello Plugin"
PLUGIN_VERSION = "1.0.0"


def on_load():
    """Called when the plugin is initialized by OpenSheet."""
    print(f"[{PLUGIN_NAME}] loaded successfully.")
    # opensheet.add_menu_item("Hello Plugin", "Say Hello", say_hello)
    # opensheet.register_formula("HELLO", hello_formula)


def on_unload():
    """Called when the plugin is disabled or the application closes."""
    print(f"[{PLUGIN_NAME}] unloaded.")


def say_hello():
    """Menu action: show a greeting in a message box."""
    # opensheet.show_message("Hello from OpenSheet Python Plugin!")
    print("Hello from OpenSheet Python Plugin!")


def hello_formula(args: list) -> str:
    """
    Custom formula: =HELLO("World") -> "Hello, World!"
    Register via opensheet.register_formula("HELLO", hello_formula)
    """
    name = str(args[0]) if args else "World"
    return f"Hello, {name}!"


# --- Entry point called by the plugin loader ---
on_load()
