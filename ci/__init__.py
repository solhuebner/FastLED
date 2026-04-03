"""FastLED CI tools and utilities

Module-level initialization is deferred to avoid paying ~20ms import cost
(global_interrupt_handler, console_utf8) on ultra-early-exit paths that only
need ci.early_exit_cache (stdlib-only). The first import of any ci.util module
that uses handle_keyboard_interrupt or console output will trigger those imports
naturally.
"""

_ci_initialized = False


def _ensure_init() -> None:
    """Lazily configure UTF-8 console on first call. Thread-safe via GIL."""
    global _ci_initialized  # noqa: PLW0603
    if _ci_initialized:
        return
    _ci_initialized = True
    try:
        from ci.util.console_utf8 import configure_utf8_console

        configure_utf8_console()
    except KeyboardInterrupt:
        import _thread  # noqa: PLC0415

        _thread.interrupt_main()
        raise
    except Exception:
        # Never fail import due to console configuration
        pass
