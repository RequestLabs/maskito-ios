

#ifndef PHITypes_h
#define PHITypes_h

#include "PHI_C_Types.h"

@interface NSValue (PhiTriangle)
+(instancetype) valueWithPhiTriangle:(PhiTriangle)value;
@property (readonly) PhiTriangle PhiTriangleValue;
@end


@interface NSValue (PhiPoint)
+(instancetype) valueWithPhiPoint:(PhiPoint)value;
@property (readonly) PhiPoint PhiPointValue;
@end

#endif /* PHITypes_h */
