

#import <Foundation/Foundation.h>
#import "PHItypes.h"

@implementation NSValue (PhiTriangle)
+(instancetype)valueWithPhiTriangle:(PhiTriangle)value {
    return [self valueWithBytes:&value objCType:@encode(PhiTriangle)];
}
- (PhiTriangle) PhiTriangleValue {
    PhiTriangle value;
    [self getValue:&value];
    return value;
}
@end

@implementation NSValue (PhiPoint)
+(instancetype)valueWithPhiPoint:(PhiPoint)value {
    return [self valueWithBytes:&value objCType:@encode(PhiPoint)];
}
- (PhiPoint) PhiPointValue {
    PhiPoint value;
    [self getValue:&value];
    return value;
}
@end
