#ifndef SAS_CONSTANTS_H
#define SAS_CONSTANTS_H

#define GL_FALSE 0
#define GL_TRUE  1

#define GL_BYTE           0x1400
#define GL_UNSIGNED_BYTE  0x1401
#define GL_SHORT          0x1402
#define GL_UNSIGNED_SHORT 0x1403
#define GL_INT            0x1404
#define GL_UNSIGNED_INT   0x1405
#define GL_FLOAT          0x1406
#define GL_2_BYTES        0x1407
#define GL_3_BYTES        0x1408
#define GL_4_BYTES        0x1409
#define GL_DOUBLE         0x140A

#define GL_POINTS     0
#define GL_TRIANGLES  4
#define GL_QUADS      7
#define GL_QUAD_STRIP 8

#define GL_NO_ERROR          0x0000
#define GL_INVALID_ENUM      0x0500
#define GL_INVALID_VALUE     0x0501
#define GL_INVALID_OPERATION 0x0502
#define GL_STACK_OVERFLOW    0x0503
#define GL_STACK_UNDERFLOW   0x0504
#define GL_OUT_OF_MEMORY     0x0505

#define GL_ATTRIB_STACK_DEPTH            0x0BB0
#define GL_CLIENT_ATTRIB_STACK_DEPTH     0x0BB1
#define GL_COLOR_CLEAR_VALUE             0x0C22
#define GL_COLOR_WRITEMASK               0x0C23
#define GL_CURRENT_INDEX                 0x0B01
#define GL_CURRENT_COLOR                 0x0B00
#define GL_CURRENT_NORMAL                0x0B02
#define GL_CURRENT_RASTER_COLOR          0x0B04
#define GL_CURRENT_RASTER_DISTANCE       0x0B09
#define GL_CURRENT_RASTER_INDEX          0x0B05
#define GL_CURRENT_RASTER_POSITION       0x0B07
#define GL_CURRENT_RASTER_TEXTURE_COORDS 0x0B06
#define GL_CURRENT_RASTER_POSITION_VALID 0x0B08
#define GL_CURRENT_TEXTURE_COORDS        0x0B03
#define GL_INDEX_CLEAR_VALUE             0x0C20
#define GL_INDEX_MODE                    0x0C30
#define GL_INDEX_WRITEMASK               0x0C21
#define GL_MODELVIEW_MATRIX              0x0BA6
#define GL_MODELVIEW_STACK_DEPTH         0x0BA3
#define GL_NAME_STACK_DEPTH              0x0D70
#define GL_PROJECTION_MATRIX             0x0BA7
#define GL_PROJECTION_STACK_DEPTH        0x0BA4
#define GL_RENDER_MODE                   0x0C40
#define GL_RGBA_MODE                     0x0C31
#define GL_TEXTURE_MATRIX                0x0BA8
#define GL_TEXTURE_STACK_DEPTH           0x0BA5
#define GL_VIEWPORT                      0x0BA2

#define GL_MATRIX_MODE 0x0BA0
#define GL_MODELVIEW   0x1700
#define GL_PROJECTION  0x1701
#define GL_TEXTURE     0x1702

#define GL_CURRENT_BIT         0x00000001
#define GL_POINT_BIT           0x00000002
#define GL_LINE_BIT            0x00000004
#define GL_POLYGON_BIT         0x00000008
#define GL_POLYGON_STIPPLE_BIT 0x00000010
#define GL_PIXEL_MODE_BIT      0x00000020
#define GL_LIGHTING_BIT        0x00000040
#define GL_FOG_BIT             0x00000080
#define GL_DEPTH_BUFFER_BIT    0x00000100
#define GL_ACCUM_BUFFER_BIT    0x00000200
#define GL_STENCIL_BUFFER_BIT  0x00000400
#define GL_VIEWPORT_BIT        0x00000800
#define GL_TRANSFORM_BIT       0x00001000
#define GL_ENABLE_BIT          0x00002000
#define GL_COLOR_BUFFER_BIT    0x00004000
#define GL_HINT_BIT            0x00008000
#define GL_EVAL_BIT            0x00010000
#define GL_LIST_BIT            0x00020000
#define GL_TEXTURE_BIT         0x00040000
#define GL_SCISSOR_BIT         0x00080000
#define GL_ALL_ATTRIB_BITS     0x000FFFFF

#define GL_NEVER      0x0200
#define GL_LESS       0x0201
#define GL_EQUAL      0x0202
#define GL_LEQUAL     0x0203
#define GL_GREATER    0x0204
#define GL_NOTEQUAL   0x0205
#define GL_GEQUAL     0x0206
#define GL_ALWAYS     0x0207
#define GL_DEPTH_TEST 0x0B71
#define GL_ALPHA_TEST 0x0BC0

#define GL_FRAGMENT_SHADER 0x8B30
#define GL_VERTEX_SHADER   0x8B31

#define GL_TEXTURE0 0x84C0
#define GL_TEXTURE1 0x84C1
#define GL_TEXTURE2 0x84C2
#define GL_TEXTURE3 0x84C3
#define GL_TEXTURE4 0x84C4
#define GL_TEXTURE5 0x84C5
#define GL_TEXTURE6 0x84C6
#define GL_TEXTURE7 0x84C7

#define GL_TEXTURE_2D 0x0DE1

#define GL_RED             0x1903
#define GL_GREEN           0x1904
#define GL_BLUE            0x1905
#define GL_ALPHA           0x1906
#define GL_RGB             0x1907
#define GL_RGBA            0x1908
#define GL_LUMINANCE       0x1909
#define GL_LUMINANCE_ALPHA 0x190A
#define GL_INTENSITY       0x8049
#define GL_BGR             0x80E0
#define GL_BGRA            0x80E1

#define GL_CULL_FACE      0x0B44
#define GL_CW             0x0900
#define GL_CCW            0x0901
#define GL_FRONT          0x0404
#define GL_BACK           0x0405
#define GL_FRONT_AND_BACK 0x0408

#define GL_SHADER_TYPE     0x8B4F
#define GL_COMPILE_STATUS  0x8B81
#define GL_LINK_STATUS     0x8B82
#define GL_INFO_LOG_LENGTH 0x8B84


#endif
