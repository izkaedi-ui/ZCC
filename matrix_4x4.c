void transform_vertex_4x4(int* mat, int vx, int vy, int vz, int vw, int* out_vec) {
    out_vec[0] = (mat[0]*vx) + (mat[1]*vy) + (mat[2]*vz) + (mat[3]*vw);
    out_vec[1] = (mat[4]*vx) + (mat[5]*vy) + (mat[6]*vz) + (mat[7]*vw);
    out_vec[2] = (mat[8]*vx) + (mat[9]*vy) + (mat[10]*vz) + (mat[11]*vw);
    out_vec[3] = (mat[12]*vx) + (mat[13]*vy) + (mat[14]*vz) + (mat[15]*vw);
}
