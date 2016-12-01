
#ifndef normalise_warp_h
#define normalise_warp_h

#include "PHI_C_Types.h"

#ifdef __cplusplus
extern "C" {
#endif
    
PhiPoint * adjusted_warp(PhiPoint * landmarks, PhiPoint * face_flat_warp, double * parameters);
    
PhiPoint * attractive_adjusted_warp(PhiPoint * landmarks, double * parameters, double * factr);
    
PhiPoint * attractive_adjusted_warp2(PhiPoint * landmarks, double * parameters, double * factr);

PhiPoint * attractive_adjusted_warp_pretty(PhiPoint * landmarks, double * parameters, double * factr);

PhiPoint * attractive_adjusted_warp_handsome(PhiPoint * landmarks, double * parameters, double * factr);
    
PhiPoint * silly_adjusted_warp(PhiPoint * landmarks, double * parameters, double * factr);
    
PhiPoint * bignose_adjusted_warp(PhiPoint * landmarks, double * parameters, double * factr);
    
PhiPoint * dynamic_adjusted_warp(PhiPoint * landmarks, double * parameters, double * factr);
    
PhiPoint * biglips_adjusted_warp(PhiPoint * landmarks, double * parameters, double * factr);
    
PhiPoint * tiny_face_warp(PhiPoint * landmarks, double * parameters, double * factr);
    
PhiPoint * face_swap_warp(PhiPoint * landmarks_face1_ptr, PhiPoint * landmarks_face2_ptr, double * parameters_face1, double * parameters_face2, double*factr_face1, double*factr_face2);
    
PhiPoint * face_puppet_warp(PhiPoint * landmarks_face1_ptr, PhiPoint * landmarks_face2_ptr, double * parameters_face1, double * parameters_face2, double*factr_face1, double*factr_face2);
    
PhiPoint3D * threed_point_set(PhiPoint * landmarks, double * parameters, double * factr );
    
PhiPoint * golden_total_pretty(PhiPoint * landmarks, double * parameters, double * factr );
   
PhiPoint * golden_total_handsome(PhiPoint * landmarks, double * parameters, double * factr );

PhiPoint * golden_inner_pretty(PhiPoint * landmarks, double * parameters, double * factr );

PhiPoint * golden_inner_handsome(PhiPoint * landmarks, double * parameters, double * factr );
    
void calc_golden_inner_pretty(PhiPoint * landmarks, double * parameters, double * factr, double * scaling);

void calc_golden_inner_handsome(PhiPoint * landmarks, double * parameters, double * factr, double * scaling);

PhiPoint * apply_golden_inner_pretty(PhiPoint * landmarks, double * parameters, double * factr, double * scaling);

PhiPoint * apply_golden_inner_handsome(PhiPoint * landmarks, double * parameters, double * factr, double * scaling);

#ifdef __cplusplus
}
#endif
#endif /* normalise_warp_h */
