#ifndef SAS_SHADER_H
#define SAS_SHADER_H

// Allocates some space for values of the varyings to be saved after executing
// to the vertex shader; the parameter sets the number of values which may
// maximally be saved (i.e., number of vertices specified before filling the
// primitive). Also, removes all previously stored values and resets the
// internal index.
void sas_varyings_alloc(size_t sz);

// Saves the current varyings' values to their respective allocated space
// specified by the current internal index, which in turn is incremented
// afterwards.
void sas_push_varyings(void);

// Resets the internal storage index.
void sas_flush_varyings(void);

// Subtracts sz from the internal storage index and moves the internal buffers
// accordingly.
void sas_flush_varyings_partially(size_t sz);

// Calculate the varyings' values before entering the fragment shader; i1, i2
// and i3 are the indizes to be used to look up the vertex values (three
// vertices, because we always draw triangles); w1, w2 and w3 are their
// respective weightings, dd is the weighting's reciprocal sum.
void sas_calc_varyings(int i1, int i2, int i3, float w1, float w2, float w3, float dd);

#endif
