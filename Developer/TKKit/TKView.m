#import "TKView.h"
#import <ncurses.h>
#import <string.h>
#import <stdlib.h>

@implementation TKView

- (id)initWithBounds:(TKRect)bounds {
	self = [super init];
	if (self) {
		_bounds = bounds;
		_title = NULL;
		_isFocused = NO;
	}
	return self;
}

- (TKRect)bounds { 
	return _bounds; 
}

- (void)setBounds:(TKRect)newBounds { 
	_bounds = newBounds;
}

- (char *)title {
	return _title;
}

- (void)setTitle:(char *)newTitle {
	if (_title) free(_title);
	_title = strdup(newTitle);
}

- (void)drawView {
	int i;

	for (i = _bounds.x; i < (_bounds.x + _bounds.width); i++) {
		mvaddch(_bounds.y, i, '-');
		mvaddch(_bounds.y + _bounds.height - 1, i, '-');
	}

	if (_title) {
		mvprintw(_bounds.y, _bounds.x + 2, "[ %s ]", _title);
	}
}

- (void)handleKeyEvent:(int)key {
	//TODO: Implement override in subclasses to intercept keystrokes
}

- (id)free {
	if (_title) {
		free(_title);
	}
	return [super free];
}
@end
