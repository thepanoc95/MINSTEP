#import <objc/Object.h>

typedef struct {
	int x, y, width, height;
} TKRect;

@interface TKView : Object
{
	TKRect _bounds;
	char *_title;
	BOOL _isFocused;
}

- (id)initWithBounds:(TKRect)bounds;

- (TKRect)bounds;
- (void)setBounds:(TKRect)newBounds;
- (char *)title;
- (void)setTitle:(char *)newTitle;

- (void)drawView;
- (void)handleKeyEvent:(int)key;

- (id)free;
@end

