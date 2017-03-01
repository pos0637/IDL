
#import "AXP/objective-c/AObject.h"
#import "AXP/objective-c/IParcelable.h"

@interface CBase : AObject<IParcelable>

@property (assign) int8_t mB;

- (void)Reset;
- (NSString*)GetTypeName;

@end

@interface CList : CBase

@property (assign) int8_t a;
@property (assign) int16_t b;
@property (assign) int64_t c;
@property (assign) double_t e;
@property (assign) Boolean bee;
@property (retain) NSNumber * f;
@property (retain) NSNumber * g;
@property (retain) NSString * m;
@property (retain) NSMutableArray * lstring;
@property (retain) NSMutableArray * list64;
@property (retain) NSMutableArray * listDouble;

- (void)Reset;
- (NSString*)GetTypeName;

@end
