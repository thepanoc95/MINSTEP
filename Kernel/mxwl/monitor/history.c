#include "monitor.h"

static char _history[MX_MONITOR_HISTORY][MX_MONITOR_MAX_LINE];
static int  _history_count = 0;
static int  _history_next  = 0;
static int  _history_pos   = -1;

int mx_monitor_history_add(const char *line)
{
    if (!line || line[0] == '\0')
        return 0;

    if (_history_count > 0 &&
        libkern_strcmp(_history[(_history_next - 1) % MX_MONITOR_HISTORY], line) == 0)
        return _history_count;

    libkern_strncpy(_history[_history_next % MX_MONITOR_HISTORY], line, MX_MONITOR_MAX_LINE);
    _history_next++;
    if (_history_count < MX_MONITOR_HISTORY)
        _history_count++;
    _history_pos = -1;

    return _history_count;
}

int mx_monitor_history_count(void)
{
    return _history_count;
}

const char *mx_monitor_history_get(int index)
{
    if (index < 0 || index >= _history_count)
        return NULL;

    int start = _history_next - _history_count;
    if (start < 0)
        start += MX_MONITOR_HISTORY;

    return _history[(start + index) % MX_MONITOR_HISTORY];
}

const char *mx_monitor_history_prev(void)
{
    if (_history_count == 0)
        return NULL;

    if (_history_pos < 0)
        _history_pos = _history_count - 1;
    else if (_history_pos > 0)
        _history_pos--;
    else
        return NULL;

    int start = _history_next - _history_count;
    if (start < 0)
        start += MX_MONITOR_HISTORY;

    return _history[(start + _history_pos) % MX_MONITOR_HISTORY];
}

const char *mx_monitor_history_next(void)
{
    if (_history_count == 0 || _history_pos < 0)
        return NULL;

    if (_history_pos >= _history_count - 1) {
        _history_pos = -1;
        return NULL;
    }

    _history_pos++;
    int start = _history_next - _history_count;
    if (start < 0)
        start += MX_MONITOR_HISTORY;

    return _history[(start + _history_pos) % MX_MONITOR_HISTORY];
}
