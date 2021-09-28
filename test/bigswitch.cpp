#include "SDL/SDL_opengl.h"
#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>

const char* WWWGLEnumToString(GLenum e) {
  switch (e) {
    case 0x0000 /* GL_POINTS */:
      return "GL_POINTS (0x0000)";
    case 0x00000001 /* GL_CURRENT_BIT */:
      return "GL_LINES/GL_CURRENT_BIT/GL_CLIENT_PIXEL_STORE_BIT (0x00000001)";
    case 0x0002 /* GL_LINE_LOOP */:
      return "GL_LINE_LOOP/GL_POINT_BIT/GL_CLIENT_VERTEX_ARRAY_BIT (0x0002)";
    case 0x0003 /* GL_LINE_STRIP */:
      return "GL_LINE_STRIP (0x0003)";
    case 0x00000004 /* GL_LINE_BIT */:
      return "GL_TRIANGLES/GL_LINE_BIT (0x00000004)";
    case 0x0005 /* GL_TRIANGLE_STRIP */:
      return "GL_TRIANGLE_STRIP (0x0005)";
    case 0x0006 /* GL_TRIANGLE_FAN */:
      return "GL_TRIANGLE_FAN (0x0006)";
    case 0x0007 /* GL_QUADS */:
      return "GL_QUADS (0x0007)";
    case 0x0008 /* GL_QUAD_STRIP */:
      return "GL_QUAD_STRIP/GL_POLYGON_BIT (0x0008)";
    case 0x0009 /* GL_POLYGON */:
      return "GL_POLYGON (0x0009)";
    case 0x00000010 /* GL_POLYGON_STIPPLE_BIT */:
      return "GL_POLYGON_STIPPLE_BIT (0x00000010)";
    case 0x00000020 /* GL_PIXEL_MODE_BIT */:
      return "GL_PIXEL_MODE_BIT (0x00000020)";
    case 0x00000040 /* GL_LIGHTING_BIT */:
      return "GL_LIGHTING_BIT (0x00000040)";
    case 0x00000080 /* GL_FOG_BIT */:
      return "GL_FOG_BIT (0x00000080)";
    case 0x00000100 /* GL_DEPTH_BUFFER_BIT */:
      return "GL_DEPTH_BUFFER_BIT/GL_ACCUM (0x00000100)";
    case 0x0101 /* GL_LOAD */:
      return "GL_LOAD (0x0101)";
    case 0x0102 /* GL_RETURN */:
      return "GL_RETURN (0x0102)";
    case 0x0103 /* GL_MULT */:
      return "GL_MULT (0x0103)";
    case 0x0104 /* GL_ADD */:
      return "GL_ADD (0x0104)";
    case 0x0200 /* GL_NEVER */:
      return "GL_NEVER/GL_ACCUM_BUFFER_BIT (0x0200)";
    case 0x0201 /* GL_LESS */:
      return "GL_LESS (0x0201)";
    case 0x0202 /* GL_EQUAL */:
      return "GL_EQUAL (0x0202)";
    case 0x0203 /* GL_LEQUAL */:
      return "GL_LEQUAL (0x0203)";
    case 0x0204 /* GL_GREATER */:
      return "GL_GREATER (0x0204)";
    case 0x0205 /* GL_NOTEQUAL */:
      return "GL_NOTEQUAL (0x0205)";
    case 0x0206 /* GL_GEQUAL */:
      return "GL_GEQUAL (0x0206)";
    case 0x0207 /* GL_ALWAYS */:
      return "GL_ALWAYS (0x0207)";
    case 0x0300 /* GL_SRC_COLOR */:
      return "GL_SRC_COLOR (0x0300)";
    case 0x0301 /* GL_ONE_MINUS_SRC_COLOR */:
      return "GL_ONE_MINUS_SRC_COLOR (0x0301)";
    case 0x0302 /* GL_SRC_ALPHA */:
      return "GL_SRC_ALPHA (0x0302)";
    case 0x0303 /* GL_ONE_MINUS_SRC_ALPHA */:
      return "GL_ONE_MINUS_SRC_ALPHA (0x0303)";
    case 0x0304 /* GL_DST_ALPHA */:
      return "GL_DST_ALPHA (0x0304)";
    case 0x0305 /* GL_ONE_MINUS_DST_ALPHA */:
      return "GL_ONE_MINUS_DST_ALPHA (0x0305)";
    case 0x0306 /* GL_DST_COLOR */:
      return "GL_DST_COLOR (0x0306)";
    case 0x0307 /* GL_ONE_MINUS_DST_COLOR */:
      return "GL_ONE_MINUS_DST_COLOR (0x0307)";
    case 0x0308 /* GL_SRC_ALPHA_SATURATE */:
      return "GL_SRC_ALPHA_SATURATE (0x0308)";
    case 0x00000400 /* GL_STENCIL_BUFFER_BIT */:
      return "GL_STENCIL_BUFFER_BIT/GL_FRONT_LEFT (0x00000400)";
    case 0x0401 /* GL_FRONT_RIGHT */:
      return "GL_FRONT_RIGHT (0x0401)";
    case 0x0402 /* GL_BACK_LEFT */:
      return "GL_BACK_LEFT (0x0402)";
    case 0x0403 /* GL_BACK_RIGHT */:
      return "GL_BACK_RIGHT (0x0403)";
    case 0x0404 /* GL_FRONT */:
      return "GL_FRONT (0x0404)";
    case 0x0405 /* GL_BACK */:
      return "GL_BACK (0x0405)";
    case 0x0406 /* GL_LEFT */:
      return "GL_LEFT (0x0406)";
    case 0x0407 /* GL_RIGHT */:
      return "GL_RIGHT (0x0407)";
    case 0x0408 /* GL_FRONT_AND_BACK */:
      return "GL_FRONT_AND_BACK (0x0408)";
    case 0x0409 /* GL_AUX0 */:
      return "GL_AUX0 (0x0409)";
    case 0x040A /* GL_AUX1 */:
      return "GL_AUX1 (0x040A)";
    case 0x040B /* GL_AUX2 */:
      return "GL_AUX2 (0x040B)";
    case 0x040C /* GL_AUX3 */:
      return "GL_AUX3 (0x040C)";
    case 0x0500 /* GL_INVALID_ENUM */:
      return "GL_INVALID_ENUM (0x0500)";
    case 0x0501 /* GL_INVALID_VALUE */:
      return "GL_INVALID_VALUE (0x0501)";
    case 0x0502 /* GL_INVALID_OPERATION */:
      return "GL_INVALID_OPERATION (0x0502)";
    case 0x0503 /* GL_STACK_OVERFLOW */:
      return "GL_STACK_OVERFLOW (0x0503)";
    case 0x0504 /* GL_STACK_UNDERFLOW */:
      return "GL_STACK_UNDERFLOW (0x0504)";
    case 0x0505 /* GL_OUT_OF_MEMORY */:
      return "GL_OUT_OF_MEMORY (0x0505)";
    case 0x0600 /* GL_2D */:
      return "GL_2D (0x0600)";
    case 0x0601 /* GL_3D */:
      return "GL_3D (0x0601)";
    case 0x0602 /* GL_3D_COLOR */:
      return "GL_3D_COLOR (0x0602)";
    case 0x0603 /* GL_3D_COLOR_TEXTURE */:
      return "GL_3D_COLOR_TEXTURE (0x0603)";
    case 0x0604 /* GL_4D_COLOR_TEXTURE */:
      return "GL_4D_COLOR_TEXTURE (0x0604)";
    case 0x0700 /* GL_PASS_THROUGH_TOKEN */:
      return "GL_PASS_THROUGH_TOKEN (0x0700)";
    case 0x0701 /* GL_POINT_TOKEN */:
      return "GL_POINT_TOKEN (0x0701)";
    case 0x0702 /* GL_LINE_TOKEN */:
      return "GL_LINE_TOKEN (0x0702)";
    case 0x0703 /* GL_POLYGON_TOKEN */:
      return "GL_POLYGON_TOKEN (0x0703)";
    case 0x0704 /* GL_BITMAP_TOKEN */:
      return "GL_BITMAP_TOKEN (0x0704)";
    case 0x0705 /* GL_DRAW_PIXEL_TOKEN */:
      return "GL_DRAW_PIXEL_TOKEN (0x0705)";
    case 0x0706 /* GL_COPY_PIXEL_TOKEN */:
      return "GL_COPY_PIXEL_TOKEN (0x0706)";
    case 0x0707 /* GL_LINE_RESET_TOKEN */:
      return "GL_LINE_RESET_TOKEN (0x0707)";
    case 0x0800 /* GL_EXP */:
      return "GL_EXP/GL_VIEWPORT_BIT (0x0800)";
    case 0x0801 /* GL_EXP2 */:
      return "GL_EXP2 (0x0801)";
    case 0x0900 /* GL_CW */:
      return "GL_CW (0x0900)";
    case 0x0901 /* GL_CCW */:
      return "GL_CCW (0x0901)";
    case 0x0A00 /* GL_COEFF */:
      return "GL_COEFF (0x0A00)";
    case 0x0A01 /* GL_ORDER */:
      return "GL_ORDER (0x0A01)";
    case 0x0A02 /* GL_DOMAIN */:
      return "GL_DOMAIN (0x0A02)";
    case 0x0B00 /* GL_CURRENT_COLOR */:
      return "GL_CURRENT_COLOR (0x0B00)";
    case 0x0B01 /* GL_CURRENT_INDEX */:
      return "GL_CURRENT_INDEX (0x0B01)";
    case 0x0B02 /* GL_CURRENT_NORMAL */:
      return "GL_CURRENT_NORMAL (0x0B02)";
    case 0x0B03 /* GL_CURRENT_TEXTURE_COORDS */:
      return "GL_CURRENT_TEXTURE_COORDS (0x0B03)";
    case 0x0B04 /* GL_CURRENT_RASTER_COLOR */:
      return "GL_CURRENT_RASTER_COLOR (0x0B04)";
    case 0x0B05 /* GL_CURRENT_RASTER_INDEX */:
      return "GL_CURRENT_RASTER_INDEX (0x0B05)";
    case 0x0B06 /* GL_CURRENT_RASTER_TEXTURE_COORDS */:
      return "GL_CURRENT_RASTER_TEXTURE_COORDS (0x0B06)";
    case 0x0B07 /* GL_CURRENT_RASTER_POSITION */:
      return "GL_CURRENT_RASTER_POSITION (0x0B07)";
    case 0x0B08 /* GL_CURRENT_RASTER_POSITION_VALID */:
      return "GL_CURRENT_RASTER_POSITION_VALID (0x0B08)";
    case 0x0B09 /* GL_CURRENT_RASTER_DISTANCE */:
      return "GL_CURRENT_RASTER_DISTANCE (0x0B09)";
    case 0x0B10 /* GL_POINT_SMOOTH */:
      return "GL_POINT_SMOOTH (0x0B10)";
    case 0x0B11 /* GL_POINT_SIZE */:
      return "GL_POINT_SIZE (0x0B11)";
    case 0x0B20 /* GL_LINE_SMOOTH */:
      return "GL_LINE_SMOOTH (0x0B20)";
    case 0x0B21 /* GL_LINE_WIDTH */:
      return "GL_LINE_WIDTH (0x0B21)";
    case 0x0B24 /* GL_LINE_STIPPLE */:
      return "GL_LINE_STIPPLE (0x0B24)";
    case 0x0B25 /* GL_LINE_STIPPLE_PATTERN */:
      return "GL_LINE_STIPPLE_PATTERN (0x0B25)";
    case 0x0B26 /* GL_LINE_STIPPLE_REPEAT */:
      return "GL_LINE_STIPPLE_REPEAT (0x0B26)";
    case 0x0B30 /* GL_LIST_MODE */:
      return "GL_LIST_MODE (0x0B30)";
    case 0x0B31 /* GL_MAX_LIST_NESTING */:
      return "GL_MAX_LIST_NESTING (0x0B31)";
    case 0x0B32 /* GL_LIST_BASE */:
      return "GL_LIST_BASE (0x0B32)";
    case 0x0B33 /* GL_LIST_INDEX */:
      return "GL_LIST_INDEX (0x0B33)";
    case 0x0B40 /* GL_POLYGON_MODE */:
      return "GL_POLYGON_MODE (0x0B40)";
    case 0x0B41 /* GL_POLYGON_SMOOTH */:
      return "GL_POLYGON_SMOOTH (0x0B41)";
    case 0x0B42 /* GL_POLYGON_STIPPLE */:
      return "GL_POLYGON_STIPPLE (0x0B42)";
    case 0x0B43 /* GL_EDGE_FLAG */:
      return "GL_EDGE_FLAG (0x0B43)";
    case 0x0B44 /* GL_CULL_FACE */:
      return "GL_CULL_FACE (0x0B44)";
    case 0x0B45 /* GL_CULL_FACE_MODE */:
      return "GL_CULL_FACE_MODE (0x0B45)";
    case 0x0B46 /* GL_FRONT_FACE */:
      return "GL_FRONT_FACE (0x0B46)";
    case 0x0B50 /* GL_LIGHTING */:
      return "GL_LIGHTING (0x0B50)";
    case 0x0B51 /* GL_LIGHT_MODEL_LOCAL_VIEWER */:
      return "GL_LIGHT_MODEL_LOCAL_VIEWER (0x0B51)";
    case 0x0B52 /* GL_LIGHT_MODEL_TWO_SIDE */:
      return "GL_LIGHT_MODEL_TWO_SIDE (0x0B52)";
    case 0x0B53 /* GL_LIGHT_MODEL_AMBIENT */:
      return "GL_LIGHT_MODEL_AMBIENT (0x0B53)";
    case 0x0B54 /* GL_SHADE_MODEL */:
      return "GL_SHADE_MODEL (0x0B54)";
    case 0x0B55 /* GL_COLOR_MATERIAL_FACE */:
      return "GL_COLOR_MATERIAL_FACE (0x0B55)";
    case 0x0B56 /* GL_COLOR_MATERIAL_PARAMETER */:
      return "GL_COLOR_MATERIAL_PARAMETER (0x0B56)";
    case 0x0B57 /* GL_COLOR_MATERIAL */:
      return "GL_COLOR_MATERIAL (0x0B57)";
    case 0x0B60 /* GL_FOG */:
      return "GL_FOG (0x0B60)";
    case 0x0B61 /* GL_FOG_INDEX */:
      return "GL_FOG_INDEX (0x0B61)";
    case 0x0B62 /* GL_FOG_DENSITY */:
      return "GL_FOG_DENSITY (0x0B62)";
    case 0x0B63 /* GL_FOG_START */:
      return "GL_FOG_START (0x0B63)";
    case 0x0B64 /* GL_FOG_END */:
      return "GL_FOG_END (0x0B64)";
    case 0x0B65 /* GL_FOG_MODE */:
      return "GL_FOG_MODE (0x0B65)";
    case 0x0B66 /* GL_FOG_COLOR */:
      return "GL_FOG_COLOR (0x0B66)";
    case 0x0B70 /* GL_DEPTH_RANGE */:
      return "GL_DEPTH_RANGE (0x0B70)";
    case 0x0B71 /* GL_DEPTH_TEST */:
      return "GL_DEPTH_TEST (0x0B71)";
    case 0x0B72 /* GL_DEPTH_WRITEMASK */:
      return "GL_DEPTH_WRITEMASK (0x0B72)";
    case 0x0B73 /* GL_DEPTH_CLEAR_VALUE */:
      return "GL_DEPTH_CLEAR_VALUE (0x0B73)";
    case 0x0B74 /* GL_DEPTH_FUNC */:
      return "GL_DEPTH_FUNC (0x0B74)";
    case 0x0B80 /* GL_ACCUM_CLEAR_VALUE */:
      return "GL_ACCUM_CLEAR_VALUE (0x0B80)";
    case 0x0B90 /* GL_STENCIL_TEST */:
      return "GL_STENCIL_TEST (0x0B90)";
    case 0x0B91 /* GL_STENCIL_CLEAR_VALUE */:
      return "GL_STENCIL_CLEAR_VALUE (0x0B91)";
    case 0x0B92 /* GL_STENCIL_FUNC */:
      return "GL_STENCIL_FUNC (0x0B92)";
    case 0x0B93 /* GL_STENCIL_VALUE_MASK */:
      return "GL_STENCIL_VALUE_MASK (0x0B93)";
    case 0x0B94 /* GL_STENCIL_FAIL */:
      return "GL_STENCIL_FAIL (0x0B94)";
    case 0x0B95 /* GL_STENCIL_PASS_DEPTH_FAIL */:
      return "GL_STENCIL_PASS_DEPTH_FAIL (0x0B95)";
    case 0x0B96 /* GL_STENCIL_PASS_DEPTH_PASS */:
      return "GL_STENCIL_PASS_DEPTH_PASS (0x0B96)";
    case 0x0B97 /* GL_STENCIL_REF */:
      return "GL_STENCIL_REF (0x0B97)";
    case 0x0B98 /* GL_STENCIL_WRITEMASK */:
      return "GL_STENCIL_WRITEMASK (0x0B98)";
    case 0x0BA0 /* GL_MATRIX_MODE */:
      return "GL_MATRIX_MODE (0x0BA0)";
    case 0x0BA1 /* GL_NORMALIZE */:
      return "GL_NORMALIZE (0x0BA1)";
    case 0x0BA2 /* GL_VIEWPORT */:
      return "GL_VIEWPORT (0x0BA2)";
    case 0x0BA3 /* GL_MODELVIEW_STACK_DEPTH */:
      return "GL_MODELVIEW_STACK_DEPTH (0x0BA3)";
    case 0x0BA4 /* GL_PROJECTION_STACK_DEPTH */:
      return "GL_PROJECTION_STACK_DEPTH (0x0BA4)";
    case 0x0BA5 /* GL_TEXTURE_STACK_DEPTH */:
      return "GL_TEXTURE_STACK_DEPTH (0x0BA5)";
    case 0x0BA6 /* GL_MODELVIEW_MATRIX */:
      return "GL_MODELVIEW_MATRIX (0x0BA6)";
    case 0x0BA7 /* GL_PROJECTION_MATRIX */:
      return "GL_PROJECTION_MATRIX (0x0BA7)";
    case 0x0BA8 /* GL_TEXTURE_MATRIX */:
      return "GL_TEXTURE_MATRIX (0x0BA8)";
    case 0x0BB0 /* GL_ATTRIB_STACK_DEPTH */:
      return "GL_ATTRIB_STACK_DEPTH (0x0BB0)";
    case 0x0BB1 /* GL_CLIENT_ATTRIB_STACK_DEPTH */:
      return "GL_CLIENT_ATTRIB_STACK_DEPTH (0x0BB1)";
    case 0x0BC0 /* GL_ALPHA_TEST */:
      return "GL_ALPHA_TEST (0x0BC0)";
    case 0x0BC1 /* GL_ALPHA_TEST_FUNC */:
      return "GL_ALPHA_TEST_FUNC (0x0BC1)";
    case 0x0BC2 /* GL_ALPHA_TEST_REF */:
      return "GL_ALPHA_TEST_REF (0x0BC2)";
    case 0x0BD0 /* GL_DITHER */:
      return "GL_DITHER (0x0BD0)";
    case 0x0BE0 /* GL_BLEND_DST */:
      return "GL_BLEND_DST (0x0BE0)";
    case 0x0BE1 /* GL_BLEND_SRC */:
      return "GL_BLEND_SRC (0x0BE1)";
    case 0x0BE2 /* GL_BLEND */:
      return "GL_BLEND (0x0BE2)";
    case 0x0BF0 /* GL_LOGIC_OP_MODE */:
      return "GL_LOGIC_OP_MODE (0x0BF0)";
    case 0x0BF1 /* GL_LOGIC_OP */:
      return "GL_LOGIC_OP/GL_INDEX_LOGIC_OP (0x0BF1)";
    // case 0x0BF1 /* GL_INDEX_LOGIC_OP */: return "GL_INDEX_LOGIC_OP (0x0BF1)";
    case 0x0BF2 /* GL_COLOR_LOGIC_OP */:
      return "GL_COLOR_LOGIC_OP (0x0BF2)";
    case 0x0C00 /* GL_AUX_BUFFERS */:
      return "GL_AUX_BUFFERS (0x0C00)";
    case 0x0C01 /* GL_DRAW_BUFFER */:
      return "GL_DRAW_BUFFER (0x0C01)";
    case 0x0C02 /* GL_READ_BUFFER */:
      return "GL_READ_BUFFER (0x0C02)";
    case 0x0C10 /* GL_SCISSOR_BOX */:
      return "GL_SCISSOR_BOX (0x0C10)";
    case 0x0C11 /* GL_SCISSOR_TEST */:
      return "GL_SCISSOR_TEST (0x0C11)";
    case 0x0C20 /* GL_INDEX_CLEAR_VALUE */:
      return "GL_INDEX_CLEAR_VALUE (0x0C20)";
    case 0x0C21 /* GL_INDEX_WRITEMASK */:
      return "GL_INDEX_WRITEMASK (0x0C21)";
    case 0x0C22 /* GL_COLOR_CLEAR_VALUE */:
      return "GL_COLOR_CLEAR_VALUE (0x0C22)";
    case 0x0C23 /* GL_COLOR_WRITEMASK */:
      return "GL_COLOR_WRITEMASK (0x0C23)";
    case 0x0C30 /* GL_INDEX_MODE */:
      return "GL_INDEX_MODE (0x0C30)";
    case 0x0C31 /* GL_RGBA_MODE */:
      return "GL_RGBA_MODE (0x0C31)";
    case 0x0C32 /* GL_DOUBLEBUFFER */:
      return "GL_DOUBLEBUFFER (0x0C32)";
    case 0x0C33 /* GL_STEREO */:
      return "GL_STEREO (0x0C33)";
    case 0x0C40 /* GL_RENDER_MODE */:
      return "GL_RENDER_MODE (0x0C40)";
    case 0x0C50 /* GL_PERSPECTIVE_CORRECTION_HINT */:
      return "GL_PERSPECTIVE_CORRECTION_HINT (0x0C50)";
    case 0x0C51 /* GL_POINT_SMOOTH_HINT */:
      return "GL_POINT_SMOOTH_HINT (0x0C51)";
    case 0x0C52 /* GL_LINE_SMOOTH_HINT */:
      return "GL_LINE_SMOOTH_HINT (0x0C52)";
    case 0x0C53 /* GL_POLYGON_SMOOTH_HINT */:
      return "GL_POLYGON_SMOOTH_HINT (0x0C53)";
    case 0x0C54 /* GL_FOG_HINT */:
      return "GL_FOG_HINT (0x0C54)";
    case 0x0C60 /* GL_TEXTURE_GEN_S */:
      return "GL_TEXTURE_GEN_S (0x0C60)";
    case 0x0C61 /* GL_TEXTURE_GEN_T */:
      return "GL_TEXTURE_GEN_T (0x0C61)";
    case 0x0C62 /* GL_TEXTURE_GEN_R */:
      return "GL_TEXTURE_GEN_R (0x0C62)";
    case 0x0C63 /* GL_TEXTURE_GEN_Q */:
      return "GL_TEXTURE_GEN_Q (0x0C63)";
    case 0x0C70 /* GL_PIXEL_MAP_I_TO_I */:
      return "GL_PIXEL_MAP_I_TO_I (0x0C70)";
    case 0x0C71 /* GL_PIXEL_MAP_S_TO_S */:
      return "GL_PIXEL_MAP_S_TO_S (0x0C71)";
    case 0x0C72 /* GL_PIXEL_MAP_I_TO_R */:
      return "GL_PIXEL_MAP_I_TO_R (0x0C72)";
    case 0x0C73 /* GL_PIXEL_MAP_I_TO_G */:
      return "GL_PIXEL_MAP_I_TO_G (0x0C73)";
    case 0x0C74 /* GL_PIXEL_MAP_I_TO_B */:
      return "GL_PIXEL_MAP_I_TO_B (0x0C74)";
    case 0x0C75 /* GL_PIXEL_MAP_I_TO_A */:
      return "GL_PIXEL_MAP_I_TO_A (0x0C75)";
    case 0x0C76 /* GL_PIXEL_MAP_R_TO_R */:
      return "GL_PIXEL_MAP_R_TO_R (0x0C76)";
    case 0x0C77 /* GL_PIXEL_MAP_G_TO_G */:
      return "GL_PIXEL_MAP_G_TO_G (0x0C77)";
    case 0x0C78 /* GL_PIXEL_MAP_B_TO_B */:
      return "GL_PIXEL_MAP_B_TO_B (0x0C78)";
    case 0x0C79 /* GL_PIXEL_MAP_A_TO_A */:
      return "GL_PIXEL_MAP_A_TO_A (0x0C79)";
    case 0x0CB0 /* GL_PIXEL_MAP_I_TO_I_SIZE */:
      return "GL_PIXEL_MAP_I_TO_I_SIZE (0x0CB0)";
    case 0x0CB1 /* GL_PIXEL_MAP_S_TO_S_SIZE */:
      return "GL_PIXEL_MAP_S_TO_S_SIZE (0x0CB1)";
    case 0x0CB2 /* GL_PIXEL_MAP_I_TO_R_SIZE */:
      return "GL_PIXEL_MAP_I_TO_R_SIZE (0x0CB2)";
    case 0x0CB3 /* GL_PIXEL_MAP_I_TO_G_SIZE */:
      return "GL_PIXEL_MAP_I_TO_G_SIZE (0x0CB3)";
    case 0x0CB4 /* GL_PIXEL_MAP_I_TO_B_SIZE */:
      return "GL_PIXEL_MAP_I_TO_B_SIZE (0x0CB4)";
    case 0x0CB5 /* GL_PIXEL_MAP_I_TO_A_SIZE */:
      return "GL_PIXEL_MAP_I_TO_A_SIZE (0x0CB5)";
    case 0x0CB6 /* GL_PIXEL_MAP_R_TO_R_SIZE */:
      return "GL_PIXEL_MAP_R_TO_R_SIZE (0x0CB6)";
    case 0x0CB7 /* GL_PIXEL_MAP_G_TO_G_SIZE */:
      return "GL_PIXEL_MAP_G_TO_G_SIZE (0x0CB7)";
    case 0x0CB8 /* GL_PIXEL_MAP_B_TO_B_SIZE */:
      return "GL_PIXEL_MAP_B_TO_B_SIZE (0x0CB8)";
    case 0x0CB9 /* GL_PIXEL_MAP_A_TO_A_SIZE */:
      return "GL_PIXEL_MAP_A_TO_A_SIZE (0x0CB9)";
    case 0x0CF0 /* GL_UNPACK_SWAP_BYTES */:
      return "GL_UNPACK_SWAP_BYTES (0x0CF0)";
    case 0x0CF1 /* GL_UNPACK_LSB_FIRST */:
      return "GL_UNPACK_LSB_FIRST (0x0CF1)";
    case 0x0CF2 /* GL_UNPACK_ROW_LENGTH */:
      return "GL_UNPACK_ROW_LENGTH (0x0CF2)";
    case 0x0CF3 /* GL_UNPACK_SKIP_ROWS */:
      return "GL_UNPACK_SKIP_ROWS (0x0CF3)";
    case 0x0CF4 /* GL_UNPACK_SKIP_PIXELS */:
      return "GL_UNPACK_SKIP_PIXELS (0x0CF4)";
    case 0x0CF5 /* GL_UNPACK_ALIGNMENT */:
      return "GL_UNPACK_ALIGNMENT (0x0CF5)";
    case 0x0D00 /* GL_PACK_SWAP_BYTES */:
      return "GL_PACK_SWAP_BYTES (0x0D00)";
    case 0x0D01 /* GL_PACK_LSB_FIRST */:
      return "GL_PACK_LSB_FIRST (0x0D01)";
    case 0x0D02 /* GL_PACK_ROW_LENGTH */:
      return "GL_PACK_ROW_LENGTH (0x0D02)";
    case 0x0D03 /* GL_PACK_SKIP_ROWS */:
      return "GL_PACK_SKIP_ROWS (0x0D03)";
    case 0x0D04 /* GL_PACK_SKIP_PIXELS */:
      return "GL_PACK_SKIP_PIXELS (0x0D04)";
    case 0x0D05 /* GL_PACK_ALIGNMENT */:
      return "GL_PACK_ALIGNMENT (0x0D05)";
    case 0x0D10 /* GL_MAP_COLOR */:
      return "GL_MAP_COLOR (0x0D10)";
    case 0x0D11 /* GL_MAP_STENCIL */:
      return "GL_MAP_STENCIL (0x0D11)";
    case 0x0D12 /* GL_INDEX_SHIFT */:
      return "GL_INDEX_SHIFT (0x0D12)";
    case 0x0D13 /* GL_INDEX_OFFSET */:
      return "GL_INDEX_OFFSET (0x0D13)";
    case 0x0D14 /* GL_RED_SCALE */:
      return "GL_RED_SCALE (0x0D14)";
    case 0x0D15 /* GL_RED_BIAS */:
      return "GL_RED_BIAS (0x0D15)";
    case 0x0D16 /* GL_ZOOM_X */:
      return "GL_ZOOM_X (0x0D16)";
    case 0x0D17 /* GL_ZOOM_Y */:
      return "GL_ZOOM_Y (0x0D17)";
    case 0x0D18 /* GL_GREEN_SCALE */:
      return "GL_GREEN_SCALE (0x0D18)";
    case 0x0D19 /* GL_GREEN_BIAS */:
      return "GL_GREEN_BIAS (0x0D19)";
    case 0x0D1A /* GL_BLUE_SCALE */:
      return "GL_BLUE_SCALE (0x0D1A)";
    case 0x0D1B /* GL_BLUE_BIAS */:
      return "GL_BLUE_BIAS (0x0D1B)";
    case 0x0D1C /* GL_ALPHA_SCALE */:
      return "GL_ALPHA_SCALE (0x0D1C)";
    case 0x0D1D /* GL_ALPHA_BIAS */:
      return "GL_ALPHA_BIAS (0x0D1D)";
    case 0x0D1E /* GL_DEPTH_SCALE */:
      return "GL_DEPTH_SCALE (0x0D1E)";
    case 0x0D1F /* GL_DEPTH_BIAS */:
      return "GL_DEPTH_BIAS (0x0D1F)";
    case 0x0D30 /* GL_MAX_EVAL_ORDER */:
      return "GL_MAX_EVAL_ORDER (0x0D30)";
    case 0x0D31 /* GL_MAX_LIGHTS */:
      return "GL_MAX_LIGHTS (0x0D31)";
    case 0x0D32 /* GL_MAX_CLIP_PLANES */:
      return "GL_MAX_CLIP_PLANES (0x0D32)";
    case 0x0D33 /* GL_MAX_TEXTURE_SIZE */:
      return "GL_MAX_TEXTURE_SIZE (0x0D33)";
    case 0x0D34 /* GL_MAX_PIXEL_MAP_TABLE */:
      return "GL_MAX_PIXEL_MAP_TABLE (0x0D34)";
    case 0x0D35 /* GL_MAX_ATTRIB_STACK_DEPTH */:
      return "GL_MAX_ATTRIB_STACK_DEPTH (0x0D35)";
    case 0x0D36 /* GL_MAX_MODELVIEW_STACK_DEPTH */:
      return "GL_MAX_MODELVIEW_STACK_DEPTH (0x0D36)";
    case 0x0D37 /* GL_MAX_NAME_STACK_DEPTH */:
      return "GL_MAX_NAME_STACK_DEPTH (0x0D37)";
    case 0x0D38 /* GL_MAX_PROJECTION_STACK_DEPTH */:
      return "GL_MAX_PROJECTION_STACK_DEPTH (0x0D38)";
    case 0x0D39 /* GL_MAX_TEXTURE_STACK_DEPTH */:
      return "GL_MAX_TEXTURE_STACK_DEPTH (0x0D39)";
    case 0x0D3A /* GL_MAX_VIEWPORT_DIMS */:
      return "GL_MAX_VIEWPORT_DIMS (0x0D3A)";
    case 0x0D3B /* GL_MAX_CLIENT_ATTRIB_STACK_DEPTH */:
      return "GL_MAX_CLIENT_ATTRIB_STACK_DEPTH (0x0D3B)";
    case 0x0D50 /* GL_SUBPIXEL_BITS */:
      return "GL_SUBPIXEL_BITS (0x0D50)";
    case 0x0D51 /* GL_INDEX_BITS */:
      return "GL_INDEX_BITS (0x0D51)";
    case 0x0D52 /* GL_RED_BITS */:
      return "GL_RED_BITS (0x0D52)";
    case 0x0D53 /* GL_GREEN_BITS */:
      return "GL_GREEN_BITS (0x0D53)";
    case 0x0D54 /* GL_BLUE_BITS */:
      return "GL_BLUE_BITS (0x0D54)";
    case 0x0D55 /* GL_ALPHA_BITS */:
      return "GL_ALPHA_BITS (0x0D55)";
    case 0x0D56 /* GL_DEPTH_BITS */:
      return "GL_DEPTH_BITS (0x0D56)";
    case 0x0D57 /* GL_STENCIL_BITS */:
      return "GL_STENCIL_BITS (0x0D57)";
    case 0x0D58 /* GL_ACCUM_RED_BITS */:
      return "GL_ACCUM_RED_BITS (0x0D58)";
    case 0x0D59 /* GL_ACCUM_GREEN_BITS */:
      return "GL_ACCUM_GREEN_BITS (0x0D59)";
    case 0x0D5A /* GL_ACCUM_BLUE_BITS */:
      return "GL_ACCUM_BLUE_BITS (0x0D5A)";
    case 0x0D5B /* GL_ACCUM_ALPHA_BITS */:
      return "GL_ACCUM_ALPHA_BITS (0x0D5B)";
    case 0x0D70 /* GL_NAME_STACK_DEPTH */:
      return "GL_NAME_STACK_DEPTH (0x0D70)";
    case 0x0D80 /* GL_AUTO_NORMAL */:
      return "GL_AUTO_NORMAL (0x0D80)";
    case 0x0D90 /* GL_MAP1_COLOR_4 */:
      return "GL_MAP1_COLOR_4 (0x0D90)";
    case 0x0D91 /* GL_MAP1_INDEX */:
      return "GL_MAP1_INDEX (0x0D91)";
    case 0x0D92 /* GL_MAP1_NORMAL */:
      return "GL_MAP1_NORMAL (0x0D92)";
    case 0x0D93 /* GL_MAP1_TEXTURE_COORD_1 */:
      return "GL_MAP1_TEXTURE_COORD_1 (0x0D93)";
    case 0x0D94 /* GL_MAP1_TEXTURE_COORD_2 */:
      return "GL_MAP1_TEXTURE_COORD_2 (0x0D94)";
    case 0x0D95 /* GL_MAP1_TEXTURE_COORD_3 */:
      return "GL_MAP1_TEXTURE_COORD_3 (0x0D95)";
    case 0x0D96 /* GL_MAP1_TEXTURE_COORD_4 */:
      return "GL_MAP1_TEXTURE_COORD_4 (0x0D96)";
    case 0x0D97 /* GL_MAP1_VERTEX_3 */:
      return "GL_MAP1_VERTEX_3 (0x0D97)";
    case 0x0D98 /* GL_MAP1_VERTEX_4 */:
      return "GL_MAP1_VERTEX_4 (0x0D98)";
    case 0x0DB0 /* GL_MAP2_COLOR_4 */:
      return "GL_MAP2_COLOR_4 (0x0DB0)";
    case 0x0DB1 /* GL_MAP2_INDEX */:
      return "GL_MAP2_INDEX (0x0DB1)";
    case 0x0DB2 /* GL_MAP2_NORMAL */:
      return "GL_MAP2_NORMAL (0x0DB2)";
    case 0x0DB3 /* GL_MAP2_TEXTURE_COORD_1 */:
      return "GL_MAP2_TEXTURE_COORD_1 (0x0DB3)";
    case 0x0DB4 /* GL_MAP2_TEXTURE_COORD_2 */:
      return "GL_MAP2_TEXTURE_COORD_2 (0x0DB4)";
    case 0x0DB5 /* GL_MAP2_TEXTURE_COORD_3 */:
      return "GL_MAP2_TEXTURE_COORD_3 (0x0DB5)";
    case 0x0DB6 /* GL_MAP2_TEXTURE_COORD_4 */:
      return "GL_MAP2_TEXTURE_COORD_4 (0x0DB6)";
    case 0x0DB7 /* GL_MAP2_VERTEX_3 */:
      return "GL_MAP2_VERTEX_3 (0x0DB7)";
    case 0x0DB8 /* GL_MAP2_VERTEX_4 */:
      return "GL_MAP2_VERTEX_4 (0x0DB8)";
    case 0x0DD0 /* GL_MAP1_GRID_DOMAIN */:
      return "GL_MAP1_GRID_DOMAIN (0x0DD0)";
    case 0x0DD1 /* GL_MAP1_GRID_SEGMENTS */:
      return "GL_MAP1_GRID_SEGMENTS (0x0DD1)";
    case 0x0DD2 /* GL_MAP2_GRID_DOMAIN */:
      return "GL_MAP2_GRID_DOMAIN (0x0DD2)";
    case 0x0DD3 /* GL_MAP2_GRID_SEGMENTS */:
      return "GL_MAP2_GRID_SEGMENTS (0x0DD3)";
    case 0x0DE0 /* GL_TEXTURE_1D */:
      return "GL_TEXTURE_1D (0x0DE0)";
    case 0x0DE1 /* GL_TEXTURE_2D */:
      return "GL_TEXTURE_2D (0x0DE1)";
    case 0x0DF0 /* GL_FEEDBACK_BUFFER_POINTER */:
      return "GL_FEEDBACK_BUFFER_POINTER (0x0DF0)";
    case 0x0DF1 /* GL_FEEDBACK_BUFFER_SIZE */:
      return "GL_FEEDBACK_BUFFER_SIZE (0x0DF1)";
    case 0x0DF2 /* GL_FEEDBACK_BUFFER_TYPE */:
      return "GL_FEEDBACK_BUFFER_TYPE (0x0DF2)";
    case 0x0DF3 /* GL_SELECTION_BUFFER_POINTER */:
      return "GL_SELECTION_BUFFER_POINTER (0x0DF3)";
    case 0x0DF4 /* GL_SELECTION_BUFFER_SIZE */:
      return "GL_SELECTION_BUFFER_SIZE (0x0DF4)";
    case 0x1000 /* GL_TEXTURE_WIDTH */:
      return "GL_TEXTURE_WIDTH/GL_TRANSFORM_BIT (0x1000)";
    case 0x1001 /* GL_TEXTURE_HEIGHT */:
      return "GL_TEXTURE_HEIGHT (0x1001)";
    // case 0x1003 /* GL_TEXTURE_COMPONENTS */: return "GL_TEXTURE_COMPONENTS
    // (0x1003)";
    case 0x1003 /* GL_TEXTURE_INTERNAL_FORMAT */:
      return "GL_TEXTURE_INTERNAL_FORMAT/GL_TEXTURE_COMPONENTS (0x1003)";
    case 0x1004 /* GL_TEXTURE_BORDER_COLOR */:
      return "GL_TEXTURE_BORDER_COLOR (0x1004)";
    case 0x1005 /* GL_TEXTURE_BORDER */:
      return "GL_TEXTURE_BORDER (0x1005)";
    case 0x1100 /* GL_DONT_CARE */:
      return "GL_DONT_CARE (0x1100)";
    case 0x1101 /* GL_FASTEST */:
      return "GL_FASTEST (0x1101)";
    case 0x1102 /* GL_NICEST */:
      return "GL_NICEST (0x1102)";
    case 0x1200 /* GL_AMBIENT */:
      return "GL_AMBIENT (0x1200)";
    case 0x1201 /* GL_DIFFUSE */:
      return "GL_DIFFUSE (0x1201)";
    case 0x1202 /* GL_SPECULAR */:
      return "GL_SPECULAR (0x1202)";
    case 0x1203 /* GL_POSITION */:
      return "GL_POSITION (0x1203)";
    case 0x1204 /* GL_SPOT_DIRECTION */:
      return "GL_SPOT_DIRECTION (0x1204)";
    case 0x1205 /* GL_SPOT_EXPONENT */:
      return "GL_SPOT_EXPONENT (0x1205)";
    case 0x1206 /* GL_SPOT_CUTOFF */:
      return "GL_SPOT_CUTOFF (0x1206)";
    case 0x1207 /* GL_CONSTANT_ATTENUATION */:
      return "GL_CONSTANT_ATTENUATION (0x1207)";
    case 0x1208 /* GL_LINEAR_ATTENUATION */:
      return "GL_LINEAR_ATTENUATION (0x1208)";
    case 0x1209 /* GL_QUADRATIC_ATTENUATION */:
      return "GL_QUADRATIC_ATTENUATION (0x1209)";
    case 0x1300 /* GL_COMPILE */:
      return "GL_COMPILE (0x1300)";
    case 0x1301 /* GL_COMPILE_AND_EXECUTE */:
      return "GL_COMPILE_AND_EXECUTE (0x1301)";
    case 0x1400 /* GL_BYTE */:
      return "GL_BYTE (0x1400)";
    case 0x1401 /* GL_UNSIGNED_BYTE */:
      return "GL_UNSIGNED_BYTE (0x1401)";
    case 0x1402 /* GL_SHORT */:
      return "GL_SHORT (0x1402)";
    case 0x1403 /* GL_UNSIGNED_SHORT */:
      return "GL_UNSIGNED_SHORT (0x1403)";
    case 0x1404 /* GL_INT */:
      return "GL_INT (0x1404)";
    case 0x1405 /* GL_UNSIGNED_INT */:
      return "GL_UNSIGNED_INT (0x1405)";
    case 0x1406 /* GL_FLOAT */:
      return "GL_FLOAT (0x1406)";
    case 0x1407 /* GL_2_BYTES */:
      return "GL_2_BYTES (0x1407)";
    case 0x1408 /* GL_3_BYTES */:
      return "GL_3_BYTES (0x1408)";
    case 0x1409 /* GL_4_BYTES */:
      return "GL_4_BYTES (0x1409)";
    case 0x140A /* GL_DOUBLE */:
      return "GL_DOUBLE (0x140A)";
    case 0x1500 /* GL_CLEAR */:
      return "GL_CLEAR (0x1500)";
    case 0x1501 /* GL_AND */:
      return "GL_AND (0x1501)";
    case 0x1502 /* GL_AND_REVERSE */:
      return "GL_AND_REVERSE (0x1502)";
    case 0x1503 /* GL_COPY */:
      return "GL_COPY (0x1503)";
    case 0x1504 /* GL_AND_INVERTED */:
      return "GL_AND_INVERTED (0x1504)";
    case 0x1505 /* GL_NOOP */:
      return "GL_NOOP (0x1505)";
    case 0x1506 /* GL_XOR */:
      return "GL_XOR (0x1506)";
    case 0x1507 /* GL_OR */:
      return "GL_OR (0x1507)";
    case 0x1508 /* GL_NOR */:
      return "GL_NOR (0x1508)";
    case 0x1509 /* GL_EQUIV */:
      return "GL_EQUIV (0x1509)";
    case 0x150A /* GL_INVERT */:
      return "GL_INVERT (0x150A)";
    case 0x150B /* GL_OR_REVERSE */:
      return "GL_OR_REVERSE (0x150B)";
    case 0x150C /* GL_COPY_INVERTED */:
      return "GL_COPY_INVERTED (0x150C)";
    case 0x150D /* GL_OR_INVERTED */:
      return "GL_OR_INVERTED (0x150D)";
    case 0x150E /* GL_NAND */:
      return "GL_NAND (0x150E)";
    case 0x150F /* GL_SET */:
      return "GL_SET (0x150F)";
    case 0x1600 /* GL_EMISSION */:
      return "GL_EMISSION (0x1600)";
    case 0x1601 /* GL_SHININESS */:
      return "GL_SHININESS (0x1601)";
    case 0x1602 /* GL_AMBIENT_AND_DIFFUSE */:
      return "GL_AMBIENT_AND_DIFFUSE (0x1602)";
    case 0x1603 /* GL_COLOR_INDEXES */:
      return "GL_COLOR_INDEXES (0x1603)";
    case 0x1700 /* GL_MODELVIEW */:
      return "GL_MODELVIEW (0x1700)";
    case 0x1701 /* GL_PROJECTION */:
      return "GL_PROJECTION (0x1701)";
    case 0x1702 /* GL_TEXTURE */:
      return "GL_TEXTURE (0x1702)";
    case 0x1800 /* GL_COLOR */:
      return "GL_COLOR (0x1800)";
    case 0x1801 /* GL_DEPTH */:
      return "GL_DEPTH (0x1801)";
    case 0x1802 /* GL_STENCIL */:
      return "GL_STENCIL (0x1802)";
    case 0x1900 /* GL_COLOR_INDEX */:
      return "GL_COLOR_INDEX (0x1900)";
    case 0x1901 /* GL_STENCIL_INDEX */:
      return "GL_STENCIL_INDEX (0x1901)";
    case 0x1902 /* GL_DEPTH_COMPONENT */:
      return "GL_DEPTH_COMPONENT (0x1902)";
    case 0x1903 /* GL_RED */:
      return "GL_RED (0x1903)";
    case 0x1904 /* GL_GREEN */:
      return "GL_GREEN (0x1904)";
    case 0x1905 /* GL_BLUE */:
      return "GL_BLUE (0x1905)";
    case 0x1906 /* GL_ALPHA */:
      return "GL_ALPHA (0x1906)";
    case 0x1907 /* GL_RGB */:
      return "GL_RGB (0x1907)";
    case 0x1908 /* GL_RGBA */:
      return "GL_RGBA (0x1908)";
    case 0x1909 /* GL_LUMINANCE */:
      return "GL_LUMINANCE (0x1909)";
    case 0x190A /* GL_LUMINANCE_ALPHA */:
      return "GL_LUMINANCE_ALPHA (0x190A)";
    case 0x1A00 /* GL_BITMAP */:
      return "GL_BITMAP (0x1A00)";
    case 0x1B00 /* GL_POINT */:
      return "GL_POINT (0x1B00)";
    case 0x1B01 /* GL_LINE */:
      return "GL_LINE (0x1B01)";
    case 0x1B02 /* GL_FILL */:
      return "GL_FILL (0x1B02)";
    case 0x1C00 /* GL_RENDER */:
      return "GL_RENDER (0x1C00)";
    case 0x1C01 /* GL_FEEDBACK */:
      return "GL_FEEDBACK (0x1C01)";
    case 0x1C02 /* GL_SELECT */:
      return "GL_SELECT (0x1C02)";
    case 0x1D00 /* GL_FLAT */:
      return "GL_FLAT (0x1D00)";
    case 0x1D01 /* GL_SMOOTH */:
      return "GL_SMOOTH (0x1D01)";
    case 0x1E00 /* GL_KEEP */:
      return "GL_KEEP (0x1E00)";
    case 0x1E01 /* GL_REPLACE */:
      return "GL_REPLACE (0x1E01)";
    case 0x1E02 /* GL_INCR */:
      return "GL_INCR (0x1E02)";
    case 0x1E03 /* GL_DECR */:
      return "GL_DECR (0x1E03)";
    case 0x1F00 /* GL_VENDOR */:
      return "GL_VENDOR (0x1F00)";
    case 0x1F01 /* GL_RENDERER */:
      return "GL_RENDERER (0x1F01)";
    case 0x1F02 /* GL_VERSION */:
      return "GL_VERSION (0x1F02)";
    case 0x1F03 /* GL_EXTENSIONS */:
      return "GL_EXTENSIONS (0x1F03)";
    case 0x2000 /* GL_S */:
      return "GL_S/GL_ENABLE_BIT (0x2000)";
    case 0x2001 /* GL_T */:
      return "GL_T (0x2001)";
    case 0x2002 /* GL_R */:
      return "GL_R (0x2002)";
    case 0x2003 /* GL_Q */:
      return "GL_Q (0x2003)";
    case 0x2100 /* GL_MODULATE */:
      return "GL_MODULATE (0x2100)";
    case 0x2101 /* GL_DECAL */:
      return "GL_DECAL (0x2101)";
    case 0x2200 /* GL_TEXTURE_ENV_MODE */:
      return "GL_TEXTURE_ENV_MODE (0x2200)";
    case 0x2201 /* GL_TEXTURE_ENV_COLOR */:
      return "GL_TEXTURE_ENV_COLOR (0x2201)";
    case 0x2300 /* GL_TEXTURE_ENV */:
      return "GL_TEXTURE_ENV (0x2300)";
    case 0x2400 /* GL_EYE_LINEAR */:
      return "GL_EYE_LINEAR (0x2400)";
    case 0x2401 /* GL_OBJECT_LINEAR */:
      return "GL_OBJECT_LINEAR (0x2401)";
    case 0x2402 /* GL_SPHERE_MAP */:
      return "GL_SPHERE_MAP (0x2402)";
    case 0x2500 /* GL_TEXTURE_GEN_MODE */:
      return "GL_TEXTURE_GEN_MODE (0x2500)";
    case 0x2501 /* GL_OBJECT_PLANE */:
      return "GL_OBJECT_PLANE (0x2501)";
    case 0x2502 /* GL_EYE_PLANE */:
      return "GL_EYE_PLANE (0x2502)";
    case 0x2600 /* GL_NEAREST */:
      return "GL_NEAREST (0x2600)";
    case 0x2601 /* GL_LINEAR */:
      return "GL_LINEAR (0x2601)";
    case 0x2700 /* GL_NEAREST_MIPMAP_NEAREST */:
      return "GL_NEAREST_MIPMAP_NEAREST (0x2700)";
    case 0x2701 /* GL_LINEAR_MIPMAP_NEAREST */:
      return "GL_LINEAR_MIPMAP_NEAREST (0x2701)";
    case 0x2702 /* GL_NEAREST_MIPMAP_LINEAR */:
      return "GL_NEAREST_MIPMAP_LINEAR (0x2702)";
    case 0x2703 /* GL_LINEAR_MIPMAP_LINEAR */:
      return "GL_LINEAR_MIPMAP_LINEAR (0x2703)";
    case 0x2800 /* GL_TEXTURE_MAG_FILTER */:
      return "GL_TEXTURE_MAG_FILTER (0x2800)";
    case 0x2801 /* GL_TEXTURE_MIN_FILTER */:
      return "GL_TEXTURE_MIN_FILTER (0x2801)";
    case 0x2802 /* GL_TEXTURE_WRAP_S */:
      return "GL_TEXTURE_WRAP_S (0x2802)";
    case 0x2803 /* GL_TEXTURE_WRAP_T */:
      return "GL_TEXTURE_WRAP_T (0x2803)";
    case 0x2900 /* GL_CLAMP */:
      return "GL_CLAMP (0x2900)";
    case 0x2901 /* GL_REPEAT */:
      return "GL_REPEAT (0x2901)";
    case 0x2A00 /* GL_POLYGON_OFFSET_UNITS */:
      return "GL_POLYGON_OFFSET_UNITS (0x2A00)";
    case 0x2A01 /* GL_POLYGON_OFFSET_POINT */:
      return "GL_POLYGON_OFFSET_POINT (0x2A01)";
    case 0x2A02 /* GL_POLYGON_OFFSET_LINE */:
      return "GL_POLYGON_OFFSET_LINE (0x2A02)";
    case 0x2A10 /* GL_R3_G3_B2 */:
      return "GL_R3_G3_B2 (0x2A10)";
    case 0x2A20 /* GL_V2F */:
      return "GL_V2F (0x2A20)";
    case 0x2A21 /* GL_V3F */:
      return "GL_V3F (0x2A21)";
    case 0x2A22 /* GL_C4UB_V2F */:
      return "GL_C4UB_V2F (0x2A22)";
    case 0x2A23 /* GL_C4UB_V3F */:
      return "GL_C4UB_V3F (0x2A23)";
    case 0x2A24 /* GL_C3F_V3F */:
      return "GL_C3F_V3F (0x2A24)";
    case 0x2A25 /* GL_N3F_V3F */:
      return "GL_N3F_V3F (0x2A25)";
    case 0x2A26 /* GL_C4F_N3F_V3F */:
      return "GL_C4F_N3F_V3F (0x2A26)";
    case 0x2A27 /* GL_T2F_V3F */:
      return "GL_T2F_V3F (0x2A27)";
    case 0x2A28 /* GL_T4F_V4F */:
      return "GL_T4F_V4F (0x2A28)";
    case 0x2A29 /* GL_T2F_C4UB_V3F */:
      return "GL_T2F_C4UB_V3F (0x2A29)";
    case 0x2A2A /* GL_T2F_C3F_V3F */:
      return "GL_T2F_C3F_V3F (0x2A2A)";
    case 0x2A2B /* GL_T2F_N3F_V3F */:
      return "GL_T2F_N3F_V3F (0x2A2B)";
    case 0x2A2C /* GL_T2F_C4F_N3F_V3F */:
      return "GL_T2F_C4F_N3F_V3F (0x2A2C)";
    case 0x2A2D /* GL_T4F_C4F_N3F_V4F */:
      return "GL_T4F_C4F_N3F_V4F (0x2A2D)";
    case 0x3000 /* GL_CLIP_PLANE0 */:
      return "GL_CLIP_PLANE0 (0x3000)";
    case 0x3001 /* GL_CLIP_PLANE1 */:
      return "GL_CLIP_PLANE1 (0x3001)";
    case 0x3002 /* GL_CLIP_PLANE2 */:
      return "GL_CLIP_PLANE2 (0x3002)";
    case 0x3003 /* GL_CLIP_PLANE3 */:
      return "GL_CLIP_PLANE3 (0x3003)";
    case 0x3004 /* GL_CLIP_PLANE4 */:
      return "GL_CLIP_PLANE4 (0x3004)";
    case 0x3005 /* GL_CLIP_PLANE5 */:
      return "GL_CLIP_PLANE5 (0x3005)";
    case 0x4000 /* GL_LIGHT0 */:
      return "GL_LIGHT0/GL_COLOR_BUFFER_BIT (0x4000)";
    case 0x4001 /* GL_LIGHT1 */:
      return "GL_LIGHT1 (0x4001)";
    case 0x4002 /* GL_LIGHT2 */:
      return "GL_LIGHT2 (0x4002)";
    case 0x4003 /* GL_LIGHT3 */:
      return "GL_LIGHT3 (0x4003)";
    case 0x4004 /* GL_LIGHT4 */:
      return "GL_LIGHT4 (0x4004)";
    case 0x4005 /* GL_LIGHT5 */:
      return "GL_LIGHT5 (0x4005)";
    case 0x4006 /* GL_LIGHT6 */:
      return "GL_LIGHT6 (0x4006)";
    case 0x4007 /* GL_LIGHT7 */:
      return "GL_LIGHT7 (0x4007)";
    case 0x00008000 /* GL_HINT_BIT */:
      return "GL_HINT_BIT (0x00008000)";
    case 0x8037 /* GL_POLYGON_OFFSET_FILL */:
      return "GL_POLYGON_OFFSET_FILL (0x8037)";
    case 0x8038 /* GL_POLYGON_OFFSET_FACTOR */:
      return "GL_POLYGON_OFFSET_FACTOR (0x8038)";
    case 0x803B /* GL_ALPHA4 */:
      return "GL_ALPHA4 (0x803B)";
    case 0x803C /* GL_ALPHA8 */:
      return "GL_ALPHA8 (0x803C)";
    case 0x803D /* GL_ALPHA12 */:
      return "GL_ALPHA12 (0x803D)";
    case 0x803E /* GL_ALPHA16 */:
      return "GL_ALPHA16 (0x803E)";
    case 0x803F /* GL_LUMINANCE4 */:
      return "GL_LUMINANCE4 (0x803F)";
    case 0x8040 /* GL_LUMINANCE8 */:
      return "GL_LUMINANCE8 (0x8040)";
    case 0x8041 /* GL_LUMINANCE12 */:
      return "GL_LUMINANCE12 (0x8041)";
    case 0x8042 /* GL_LUMINANCE16 */:
      return "GL_LUMINANCE16 (0x8042)";
    case 0x8043 /* GL_LUMINANCE4_ALPHA4 */:
      return "GL_LUMINANCE4_ALPHA4 (0x8043)";
    case 0x8044 /* GL_LUMINANCE6_ALPHA2 */:
      return "GL_LUMINANCE6_ALPHA2 (0x8044)";
    case 0x8045 /* GL_LUMINANCE8_ALPHA8 */:
      return "GL_LUMINANCE8_ALPHA8 (0x8045)";
    case 0x8046 /* GL_LUMINANCE12_ALPHA4 */:
      return "GL_LUMINANCE12_ALPHA4 (0x8046)";
    case 0x8047 /* GL_LUMINANCE12_ALPHA12 */:
      return "GL_LUMINANCE12_ALPHA12 (0x8047)";
    case 0x8048 /* GL_LUMINANCE16_ALPHA16 */:
      return "GL_LUMINANCE16_ALPHA16 (0x8048)";
    case 0x8049 /* GL_INTENSITY */:
      return "GL_INTENSITY (0x8049)";
    case 0x804A /* GL_INTENSITY4 */:
      return "GL_INTENSITY4 (0x804A)";
    case 0x804B /* GL_INTENSITY8 */:
      return "GL_INTENSITY8 (0x804B)";
    case 0x804C /* GL_INTENSITY12 */:
      return "GL_INTENSITY12 (0x804C)";
    case 0x804D /* GL_INTENSITY16 */:
      return "GL_INTENSITY16 (0x804D)";
    case 0x804F /* GL_RGB4 */:
      return "GL_RGB4 (0x804F)";
    case 0x8050 /* GL_RGB5 */:
      return "GL_RGB5 (0x8050)";
    case 0x8051 /* GL_RGB8 */:
      return "GL_RGB8 (0x8051)";
    case 0x8052 /* GL_RGB10 */:
      return "GL_RGB10 (0x8052)";
    case 0x8053 /* GL_RGB12 */:
      return "GL_RGB12 (0x8053)";
    case 0x8054 /* GL_RGB16 */:
      return "GL_RGB16 (0x8054)";
    case 0x8055 /* GL_RGBA2 */:
      return "GL_RGBA2 (0x8055)";
    case 0x8056 /* GL_RGBA4 */:
      return "GL_RGBA4 (0x8056)";
    case 0x8057 /* GL_RGB5_A1 */:
      return "GL_RGB5_A1 (0x8057)";
    case 0x8058 /* GL_RGBA8 */:
      return "GL_RGBA8 (0x8058)";
    case 0x8059 /* GL_RGB10_A2 */:
      return "GL_RGB10_A2 (0x8059)";
    case 0x805A /* GL_RGBA12 */:
      return "GL_RGBA12 (0x805A)";
    case 0x805B /* GL_RGBA16 */:
      return "GL_RGBA16 (0x805B)";
    case 0x805C /* GL_TEXTURE_RED_SIZE */:
      return "GL_TEXTURE_RED_SIZE (0x805C)";
    case 0x805D /* GL_TEXTURE_GREEN_SIZE */:
      return "GL_TEXTURE_GREEN_SIZE (0x805D)";
    case 0x805E /* GL_TEXTURE_BLUE_SIZE */:
      return "GL_TEXTURE_BLUE_SIZE (0x805E)";
    case 0x805F /* GL_TEXTURE_ALPHA_SIZE */:
      return "GL_TEXTURE_ALPHA_SIZE (0x805F)";
    case 0x8060 /* GL_TEXTURE_LUMINANCE_SIZE */:
      return "GL_TEXTURE_LUMINANCE_SIZE (0x8060)";
    case 0x8061 /* GL_TEXTURE_INTENSITY_SIZE */:
      return "GL_TEXTURE_INTENSITY_SIZE (0x8061)";
    case 0x8063 /* GL_PROXY_TEXTURE_1D */:
      return "GL_PROXY_TEXTURE_1D (0x8063)";
    case 0x8064 /* GL_PROXY_TEXTURE_2D */:
      return "GL_PROXY_TEXTURE_2D (0x8064)";
    case 0x8066 /* GL_TEXTURE_PRIORITY */:
      return "GL_TEXTURE_PRIORITY (0x8066)";
    case 0x8067 /* GL_TEXTURE_RESIDENT */:
      return "GL_TEXTURE_RESIDENT (0x8067)";
    case 0x8068 /* GL_TEXTURE_BINDING_1D */:
      return "GL_TEXTURE_BINDING_1D (0x8068)";
    case 0x8069 /* GL_TEXTURE_BINDING_2D */:
      return "GL_TEXTURE_BINDING_2D (0x8069)";
    case 0x8074 /* GL_VERTEX_ARRAY */:
      return "GL_VERTEX_ARRAY (0x8074)";
    case 0x8075 /* GL_NORMAL_ARRAY */:
      return "GL_NORMAL_ARRAY (0x8075)";
    case 0x8076 /* GL_COLOR_ARRAY */:
      return "GL_COLOR_ARRAY (0x8076)";
    case 0x8077 /* GL_INDEX_ARRAY */:
      return "GL_INDEX_ARRAY (0x8077)";
    case 0x8078 /* GL_TEXTURE_COORD_ARRAY */:
      return "GL_TEXTURE_COORD_ARRAY (0x8078)";
    case 0x8079 /* GL_EDGE_FLAG_ARRAY */:
      return "GL_EDGE_FLAG_ARRAY (0x8079)";
    case 0x807A /* GL_VERTEX_ARRAY_SIZE */:
      return "GL_VERTEX_ARRAY_SIZE (0x807A)";
    case 0x807B /* GL_VERTEX_ARRAY_TYPE */:
      return "GL_VERTEX_ARRAY_TYPE (0x807B)";
    case 0x807C /* GL_VERTEX_ARRAY_STRIDE */:
      return "GL_VERTEX_ARRAY_STRIDE (0x807C)";
    case 0x807E /* GL_NORMAL_ARRAY_TYPE */:
      return "GL_NORMAL_ARRAY_TYPE (0x807E)";
    case 0x807F /* GL_NORMAL_ARRAY_STRIDE */:
      return "GL_NORMAL_ARRAY_STRIDE (0x807F)";
    case 0x8081 /* GL_COLOR_ARRAY_SIZE */:
      return "GL_COLOR_ARRAY_SIZE (0x8081)";
    case 0x8082 /* GL_COLOR_ARRAY_TYPE */:
      return "GL_COLOR_ARRAY_TYPE (0x8082)";
    case 0x8083 /* GL_COLOR_ARRAY_STRIDE */:
      return "GL_COLOR_ARRAY_STRIDE (0x8083)";
    case 0x8085 /* GL_INDEX_ARRAY_TYPE */:
      return "GL_INDEX_ARRAY_TYPE (0x8085)";
    case 0x8086 /* GL_INDEX_ARRAY_STRIDE */:
      return "GL_INDEX_ARRAY_STRIDE (0x8086)";
    case 0x8088 /* GL_TEXTURE_COORD_ARRAY_SIZE */:
      return "GL_TEXTURE_COORD_ARRAY_SIZE (0x8088)";
    case 0x8089 /* GL_TEXTURE_COORD_ARRAY_TYPE */:
      return "GL_TEXTURE_COORD_ARRAY_TYPE (0x8089)";
    case 0x808A /* GL_TEXTURE_COORD_ARRAY_STRIDE */:
      return "GL_TEXTURE_COORD_ARRAY_STRIDE (0x808A)";
    case 0x808C /* GL_EDGE_FLAG_ARRAY_STRIDE */:
      return "GL_EDGE_FLAG_ARRAY_STRIDE (0x808C)";
    case 0x808E /* GL_VERTEX_ARRAY_POINTER */:
      return "GL_VERTEX_ARRAY_POINTER (0x808E)";
    case 0x808F /* GL_NORMAL_ARRAY_POINTER */:
      return "GL_NORMAL_ARRAY_POINTER (0x808F)";
    case 0x8090 /* GL_COLOR_ARRAY_POINTER */:
      return "GL_COLOR_ARRAY_POINTER (0x8090)";
    case 0x8091 /* GL_INDEX_ARRAY_POINTER */:
      return "GL_INDEX_ARRAY_POINTER (0x8091)";
    case 0x8092 /* GL_TEXTURE_COORD_ARRAY_POINTER */:
      return "GL_TEXTURE_COORD_ARRAY_POINTER (0x8092)";
    case 0x8093 /* GL_EDGE_FLAG_ARRAY_POINTER */:
      return "GL_EDGE_FLAG_ARRAY_POINTER (0x8093)";
    case 0x80E2 /* GL_COLOR_INDEX1_EXT */:
      return "GL_COLOR_INDEX1_EXT (0x80E2)";
    case 0x80E3 /* GL_COLOR_INDEX2_EXT */:
      return "GL_COLOR_INDEX2_EXT (0x80E3)";
    case 0x80E4 /* GL_COLOR_INDEX4_EXT */:
      return "GL_COLOR_INDEX4_EXT (0x80E4)";
    case 0x80E5 /* GL_COLOR_INDEX8_EXT */:
      return "GL_COLOR_INDEX8_EXT (0x80E5)";
    case 0x80E6 /* GL_COLOR_INDEX12_EXT */:
      return "GL_COLOR_INDEX12_EXT (0x80E6)";
    case 0x80E7 /* GL_COLOR_INDEX16_EXT */:
      return "GL_COLOR_INDEX16_EXT (0x80E7)";
    case 0x00010000 /* GL_EVAL_BIT */:
      return "GL_EVAL_BIT (0x00010000)";
    case 0x00020000 /* GL_LIST_BIT */:
      return "GL_LIST_BIT (0x00020000)";
    case 0x00040000 /* GL_TEXTURE_BIT */:
      return "GL_TEXTURE_BIT (0x00040000)";
    case 0x00080000 /* GL_SCISSOR_BIT */:
      return "GL_SCISSOR_BIT (0x00080000)";
    case 0x000fffff /* GL_ALL_ATTRIB_BITS */:
      return "GL_ALL_ATTRIB_BITS (0x000fffff)";
    case 0xffffffff /* GL_CLIENT_ALL_ATTRIB_BITS */:
      return "GL_CLIENT_ALL_ATTRIB_BITS (0xffffffff)";

    // case 0x0B12 /* GL_SMOOTH_POINT_SIZE_RANGE */: return
    // "GL_SMOOTH_POINT_SIZE_RANGE (0x0B12)";
    case 0x0B12 /* GL_POINT_SIZE_RANGE */:
      return "GL_POINT_SIZE_RANGE/GL_SMOOTH_POINT_SIZE_RANGE (0x0B12)";
    // case 0x0B13 /* GL_SMOOTH_POINT_SIZE_GRANULARITY */: return
    // "GL_SMOOTH_POINT_SIZE_GRANULARITY (0x0B13)";
    case 0x0B13 /* GL_POINT_SIZE_GRANULARITY */:
      return "GL_POINT_SIZE_GRANULARITY/GL_SMOOTH_POINT_SIZE_GRANULARITY "
             "(0x0B13)";
    case 0x0B22 /* GL_LINE_WIDTH_RANGE */:
      return "GL_LINE_WIDTH_RANGE/GL_SMOOTH_LINE_WIDTH_RANGE (0x0B22)";
    // case 0x0B22 /* GL_SMOOTH_LINE_WIDTH_RANGE */: return
    // "GL_SMOOTH_LINE_WIDTH_RANGE (0x0B22)"; case 0x0B23 /*
    // GL_SMOOTH_LINE_WIDTH_GRANULARITY */: return
    // "GL_SMOOTH_LINE_WIDTH_GRANULARITY (0x0B23)";
    case 0x0B23 /* GL_LINE_WIDTH_GRANULARITY */:
      return "GL_LINE_WIDTH_GRANULARITY/GL_SMOOTH_LINE_WIDTH_GRANULARITY "
             "(0x0B23)";
    case 0x8032 /* GL_UNSIGNED_BYTE_3_3_2 */:
      return "GL_UNSIGNED_BYTE_3_3_2 (0x8032)";
    case 0x8033 /* GL_UNSIGNED_SHORT_4_4_4_4 */:
      return "GL_UNSIGNED_SHORT_4_4_4_4 (0x8033)";
    case 0x8034 /* GL_UNSIGNED_SHORT_5_5_5_1 */:
      return "GL_UNSIGNED_SHORT_5_5_5_1 (0x8034)";
    case 0x8035 /* GL_UNSIGNED_INT_8_8_8_8 */:
      return "GL_UNSIGNED_INT_8_8_8_8 (0x8035)";
    case 0x8036 /* GL_UNSIGNED_INT_10_10_10_2 */:
      return "GL_UNSIGNED_INT_10_10_10_2 (0x8036)";
    case 0x803A /* GL_RESCALE_NORMAL */:
      return "GL_RESCALE_NORMAL (0x803A)";
    case 0x806A /* GL_TEXTURE_BINDING_3D */:
      return "GL_TEXTURE_BINDING_3D (0x806A)";
    case 0x806B /* GL_PACK_SKIP_IMAGES */:
      return "GL_PACK_SKIP_IMAGES (0x806B)";
    case 0x806C /* GL_PACK_IMAGE_HEIGHT */:
      return "GL_PACK_IMAGE_HEIGHT (0x806C)";
    case 0x806D /* GL_UNPACK_SKIP_IMAGES */:
      return "GL_UNPACK_SKIP_IMAGES (0x806D)";
    case 0x806E /* GL_UNPACK_IMAGE_HEIGHT */:
      return "GL_UNPACK_IMAGE_HEIGHT (0x806E)";
    case 0x806F /* GL_TEXTURE_3D */:
      return "GL_TEXTURE_3D (0x806F)";
    case 0x8070 /* GL_PROXY_TEXTURE_3D */:
      return "GL_PROXY_TEXTURE_3D (0x8070)";
    case 0x8071 /* GL_TEXTURE_DEPTH */:
      return "GL_TEXTURE_DEPTH (0x8071)";
    case 0x8072 /* GL_TEXTURE_WRAP_R */:
      return "GL_TEXTURE_WRAP_R (0x8072)";
    case 0x8073 /* GL_MAX_3D_TEXTURE_SIZE */:
      return "GL_MAX_3D_TEXTURE_SIZE (0x8073)";
    case 0x80E0 /* GL_BGR */:
      return "GL_BGR (0x80E0)";
    case 0x80E1 /* GL_BGRA */:
      return "GL_BGRA (0x80E1)";
    case 0x80E8 /* GL_MAX_ELEMENTS_VERTICES */:
      return "GL_MAX_ELEMENTS_VERTICES (0x80E8)";
    case 0x80E9 /* GL_MAX_ELEMENTS_INDICES */:
      return "GL_MAX_ELEMENTS_INDICES (0x80E9)";
    case 0x812F /* GL_CLAMP_TO_EDGE */:
      return "GL_CLAMP_TO_EDGE (0x812F)";
    case 0x813A /* GL_TEXTURE_MIN_LOD */:
      return "GL_TEXTURE_MIN_LOD (0x813A)";
    case 0x813B /* GL_TEXTURE_MAX_LOD */:
      return "GL_TEXTURE_MAX_LOD (0x813B)";
    case 0x813C /* GL_TEXTURE_BASE_LEVEL */:
      return "GL_TEXTURE_BASE_LEVEL (0x813C)";
    case 0x813D /* GL_TEXTURE_MAX_LEVEL */:
      return "GL_TEXTURE_MAX_LEVEL (0x813D)";
    case 0x81F8 /* GL_LIGHT_MODEL_COLOR_CONTROL */:
      return "GL_LIGHT_MODEL_COLOR_CONTROL (0x81F8)";
    case 0x81F9 /* GL_SINGLE_COLOR */:
      return "GL_SINGLE_COLOR (0x81F9)";
    case 0x81FA /* GL_SEPARATE_SPECULAR_COLOR */:
      return "GL_SEPARATE_SPECULAR_COLOR (0x81FA)";
    case 0x8362 /* GL_UNSIGNED_BYTE_2_3_3_REV */:
      return "GL_UNSIGNED_BYTE_2_3_3_REV (0x8362)";
    case 0x8363 /* GL_UNSIGNED_SHORT_5_6_5 */:
      return "GL_UNSIGNED_SHORT_5_6_5 (0x8363)";
    case 0x8364 /* GL_UNSIGNED_SHORT_5_6_5_REV */:
      return "GL_UNSIGNED_SHORT_5_6_5_REV (0x8364)";
    case 0x8365 /* GL_UNSIGNED_SHORT_4_4_4_4_REV */:
      return "GL_UNSIGNED_SHORT_4_4_4_4_REV (0x8365)";
    case 0x8366 /* GL_UNSIGNED_SHORT_1_5_5_5_REV */:
      return "GL_UNSIGNED_SHORT_1_5_5_5_REV (0x8366)";
    case 0x8367 /* GL_UNSIGNED_INT_8_8_8_8_REV */:
      return "GL_UNSIGNED_INT_8_8_8_8_REV (0x8367)";
    case 0x8368 /* GL_UNSIGNED_INT_2_10_10_10_REV */:
      return "GL_UNSIGNED_INT_2_10_10_10_REV (0x8368)";
    case 0x846D /* GL_ALIASED_POINT_SIZE_RANGE */:
      return "GL_ALIASED_POINT_SIZE_RANGE (0x846D)";
    case 0x846E /* GL_ALIASED_LINE_WIDTH_RANGE */:
      return "GL_ALIASED_LINE_WIDTH_RANGE (0x846E)";

    case 0x809D /* GL_MULTISAMPLE */:
      return "GL_MULTISAMPLE (0x809D)";
    case 0x809E /* GL_SAMPLE_ALPHA_TO_COVERAGE */:
      return "GL_SAMPLE_ALPHA_TO_COVERAGE (0x809E)";
    case 0x809F /* GL_SAMPLE_ALPHA_TO_ONE */:
      return "GL_SAMPLE_ALPHA_TO_ONE (0x809F)";
    case 0x80A0 /* GL_SAMPLE_COVERAGE */:
      return "GL_SAMPLE_COVERAGE (0x80A0)";
    case 0x80A8 /* GL_SAMPLE_BUFFERS */:
      return "GL_SAMPLE_BUFFERS (0x80A8)";
    case 0x80A9 /* GL_SAMPLES */:
      return "GL_SAMPLES (0x80A9)";
    case 0x80AA /* GL_SAMPLE_COVERAGE_VALUE */:
      return "GL_SAMPLE_COVERAGE_VALUE (0x80AA)";
    case 0x80AB /* GL_SAMPLE_COVERAGE_INVERT */:
      return "GL_SAMPLE_COVERAGE_INVERT (0x80AB)";
    case 0x812D /* GL_CLAMP_TO_BORDER */:
      return "GL_CLAMP_TO_BORDER (0x812D)";
    case 0x84C0 /* GL_TEXTURE0 */:
      return "GL_TEXTURE0 (0x84C0)";
    case 0x84C1 /* GL_TEXTURE1 */:
      return "GL_TEXTURE1 (0x84C1)";
    case 0x84C2 /* GL_TEXTURE2 */:
      return "GL_TEXTURE2 (0x84C2)";
    case 0x84C3 /* GL_TEXTURE3 */:
      return "GL_TEXTURE3 (0x84C3)";
    case 0x84C4 /* GL_TEXTURE4 */:
      return "GL_TEXTURE4 (0x84C4)";
    case 0x84C5 /* GL_TEXTURE5 */:
      return "GL_TEXTURE5 (0x84C5)";
    case 0x84C6 /* GL_TEXTURE6 */:
      return "GL_TEXTURE6 (0x84C6)";
    case 0x84C7 /* GL_TEXTURE7 */:
      return "GL_TEXTURE7 (0x84C7)";
    case 0x84C8 /* GL_TEXTURE8 */:
      return "GL_TEXTURE8 (0x84C8)";
    case 0x84C9 /* GL_TEXTURE9 */:
      return "GL_TEXTURE9 (0x84C9)";
    case 0x84CA /* GL_TEXTURE10 */:
      return "GL_TEXTURE10 (0x84CA)";
    case 0x84CB /* GL_TEXTURE11 */:
      return "GL_TEXTURE11 (0x84CB)";
    case 0x84CC /* GL_TEXTURE12 */:
      return "GL_TEXTURE12 (0x84CC)";
    case 0x84CD /* GL_TEXTURE13 */:
      return "GL_TEXTURE13 (0x84CD)";
    case 0x84CE /* GL_TEXTURE14 */:
      return "GL_TEXTURE14 (0x84CE)";
    case 0x84CF /* GL_TEXTURE15 */:
      return "GL_TEXTURE15 (0x84CF)";
    case 0x84D0 /* GL_TEXTURE16 */:
      return "GL_TEXTURE16 (0x84D0)";
    case 0x84D1 /* GL_TEXTURE17 */:
      return "GL_TEXTURE17 (0x84D1)";
    case 0x84D2 /* GL_TEXTURE18 */:
      return "GL_TEXTURE18 (0x84D2)";
    case 0x84D3 /* GL_TEXTURE19 */:
      return "GL_TEXTURE19 (0x84D3)";
    case 0x84D4 /* GL_TEXTURE20 */:
      return "GL_TEXTURE20 (0x84D4)";
    case 0x84D5 /* GL_TEXTURE21 */:
      return "GL_TEXTURE21 (0x84D5)";
    case 0x84D6 /* GL_TEXTURE22 */:
      return "GL_TEXTURE22 (0x84D6)";
    case 0x84D7 /* GL_TEXTURE23 */:
      return "GL_TEXTURE23 (0x84D7)";
    case 0x84D8 /* GL_TEXTURE24 */:
      return "GL_TEXTURE24 (0x84D8)";
    case 0x84D9 /* GL_TEXTURE25 */:
      return "GL_TEXTURE25 (0x84D9)";
    case 0x84DA /* GL_TEXTURE26 */:
      return "GL_TEXTURE26 (0x84DA)";
    case 0x84DB /* GL_TEXTURE27 */:
      return "GL_TEXTURE27 (0x84DB)";
    case 0x84DC /* GL_TEXTURE28 */:
      return "GL_TEXTURE28 (0x84DC)";
    case 0x84DD /* GL_TEXTURE29 */:
      return "GL_TEXTURE29 (0x84DD)";
    case 0x84DE /* GL_TEXTURE30 */:
      return "GL_TEXTURE30 (0x84DE)";
    case 0x84DF /* GL_TEXTURE31 */:
      return "GL_TEXTURE31 (0x84DF)";
    case 0x84E0 /* GL_ACTIVE_TEXTURE */:
      return "GL_ACTIVE_TEXTURE (0x84E0)";
    case 0x84E1 /* GL_CLIENT_ACTIVE_TEXTURE */:
      return "GL_CLIENT_ACTIVE_TEXTURE (0x84E1)";
    case 0x84E2 /* GL_MAX_TEXTURE_UNITS */:
      return "GL_MAX_TEXTURE_UNITS (0x84E2)";
    case 0x84E3 /* GL_TRANSPOSE_MODELVIEW_MATRIX */:
      return "GL_TRANSPOSE_MODELVIEW_MATRIX (0x84E3)";
    case 0x84E4 /* GL_TRANSPOSE_PROJECTION_MATRIX */:
      return "GL_TRANSPOSE_PROJECTION_MATRIX (0x84E4)";
    case 0x84E5 /* GL_TRANSPOSE_TEXTURE_MATRIX */:
      return "GL_TRANSPOSE_TEXTURE_MATRIX (0x84E5)";
    case 0x84E6 /* GL_TRANSPOSE_COLOR_MATRIX */:
      return "GL_TRANSPOSE_COLOR_MATRIX (0x84E6)";
    case 0x84E7 /* GL_SUBTRACT */:
      return "GL_SUBTRACT (0x84E7)";
    case 0x84E9 /* GL_COMPRESSED_ALPHA */:
      return "GL_COMPRESSED_ALPHA (0x84E9)";
    case 0x84EA /* GL_COMPRESSED_LUMINANCE */:
      return "GL_COMPRESSED_LUMINANCE (0x84EA)";
    case 0x84EB /* GL_COMPRESSED_LUMINANCE_ALPHA */:
      return "GL_COMPRESSED_LUMINANCE_ALPHA (0x84EB)";
    case 0x84EC /* GL_COMPRESSED_INTENSITY */:
      return "GL_COMPRESSED_INTENSITY (0x84EC)";
    case 0x84ED /* GL_COMPRESSED_RGB */:
      return "GL_COMPRESSED_RGB (0x84ED)";
    case 0x84EE /* GL_COMPRESSED_RGBA */:
      return "GL_COMPRESSED_RGBA (0x84EE)";
    case 0x84EF /* GL_TEXTURE_COMPRESSION_HINT */:
      return "GL_TEXTURE_COMPRESSION_HINT (0x84EF)";
    case 0x8511 /* GL_NORMAL_MAP */:
      return "GL_NORMAL_MAP (0x8511)";
    case 0x8512 /* GL_REFLECTION_MAP */:
      return "GL_REFLECTION_MAP (0x8512)";
    case 0x8513 /* GL_TEXTURE_CUBE_MAP */:
      return "GL_TEXTURE_CUBE_MAP (0x8513)";
    case 0x8514 /* GL_TEXTURE_BINDING_CUBE_MAP */:
      return "GL_TEXTURE_BINDING_CUBE_MAP (0x8514)";
    case 0x8515 /* GL_TEXTURE_CUBE_MAP_POSITIVE_X */:
      return "GL_TEXTURE_CUBE_MAP_POSITIVE_X (0x8515)";
    case 0x8516 /* GL_TEXTURE_CUBE_MAP_NEGATIVE_X */:
      return "GL_TEXTURE_CUBE_MAP_NEGATIVE_X (0x8516)";
    case 0x8517 /* GL_TEXTURE_CUBE_MAP_POSITIVE_Y */:
      return "GL_TEXTURE_CUBE_MAP_POSITIVE_Y (0x8517)";
    case 0x8518 /* GL_TEXTURE_CUBE_MAP_NEGATIVE_Y */:
      return "GL_TEXTURE_CUBE_MAP_NEGATIVE_Y (0x8518)";
    case 0x8519 /* GL_TEXTURE_CUBE_MAP_POSITIVE_Z */:
      return "GL_TEXTURE_CUBE_MAP_POSITIVE_Z (0x8519)";
    case 0x851A /* GL_TEXTURE_CUBE_MAP_NEGATIVE_Z */:
      return "GL_TEXTURE_CUBE_MAP_NEGATIVE_Z (0x851A)";
    case 0x851B /* GL_PROXY_TEXTURE_CUBE_MAP */:
      return "GL_PROXY_TEXTURE_CUBE_MAP (0x851B)";
    case 0x851C /* GL_MAX_CUBE_MAP_TEXTURE_SIZE */:
      return "GL_MAX_CUBE_MAP_TEXTURE_SIZE (0x851C)";
    case 0x8570 /* GL_COMBINE */:
      return "GL_COMBINE (0x8570)";
    case 0x8571 /* GL_COMBINE_RGB */:
      return "GL_COMBINE_RGB (0x8571)";
    case 0x8572 /* GL_COMBINE_ALPHA */:
      return "GL_COMBINE_ALPHA (0x8572)";
    case 0x8573 /* GL_RGB_SCALE */:
      return "GL_RGB_SCALE (0x8573)";
    case 0x8574 /* GL_ADD_SIGNED */:
      return "GL_ADD_SIGNED (0x8574)";
    case 0x8575 /* GL_INTERPOLATE */:
      return "GL_INTERPOLATE (0x8575)";
    case 0x8576 /* GL_CONSTANT */:
      return "GL_CONSTANT (0x8576)";
    case 0x8577 /* GL_PRIMARY_COLOR */:
      return "GL_PRIMARY_COLOR (0x8577)";
    case 0x8578 /* GL_PREVIOUS */:
      return "GL_PREVIOUS (0x8578)";
    case 0x8580 /* GL_SOURCE0_RGB */:
      return "GL_SOURCE0_RGB (0x8580)";
    case 0x8581 /* GL_SOURCE1_RGB */:
      return "GL_SOURCE1_RGB (0x8581)";
    case 0x8582 /* GL_SOURCE2_RGB */:
      return "GL_SOURCE2_RGB (0x8582)";
    case 0x8588 /* GL_SOURCE0_ALPHA */:
      return "GL_SOURCE0_ALPHA (0x8588)";
    case 0x8589 /* GL_SOURCE1_ALPHA */:
      return "GL_SOURCE1_ALPHA (0x8589)";
    case 0x858A /* GL_SOURCE2_ALPHA */:
      return "GL_SOURCE2_ALPHA (0x858A)";
    case 0x8590 /* GL_OPERAND0_RGB */:
      return "GL_OPERAND0_RGB (0x8590)";
    case 0x8591 /* GL_OPERAND1_RGB */:
      return "GL_OPERAND1_RGB (0x8591)";
    case 0x8592 /* GL_OPERAND2_RGB */:
      return "GL_OPERAND2_RGB (0x8592)";
    case 0x8598 /* GL_OPERAND0_ALPHA */:
      return "GL_OPERAND0_ALPHA (0x8598)";
    case 0x8599 /* GL_OPERAND1_ALPHA */:
      return "GL_OPERAND1_ALPHA (0x8599)";
    case 0x859A /* GL_OPERAND2_ALPHA */:
      return "GL_OPERAND2_ALPHA (0x859A)";
    case 0x86A0 /* GL_TEXTURE_COMPRESSED_IMAGE_SIZE */:
      return "GL_TEXTURE_COMPRESSED_IMAGE_SIZE (0x86A0)";
    case 0x86A1 /* GL_TEXTURE_COMPRESSED */:
      return "GL_TEXTURE_COMPRESSED (0x86A1)";
    case 0x86A2 /* GL_NUM_COMPRESSED_TEXTURE_FORMATS */:
      return "GL_NUM_COMPRESSED_TEXTURE_FORMATS (0x86A2)";
    case 0x86A3 /* GL_COMPRESSED_TEXTURE_FORMATS */:
      return "GL_COMPRESSED_TEXTURE_FORMATS (0x86A3)";
    case 0x86AE /* GL_DOT3_RGB */:
      return "GL_DOT3_RGB (0x86AE)";
    case 0x86AF /* GL_DOT3_RGBA */:
      return "GL_DOT3_RGBA (0x86AF)";
    case 0x20000000 /* GL_MULTISAMPLE_BIT */:
      return "GL_MULTISAMPLE_BIT (0x20000000)";

    case 0x80C8 /* GL_BLEND_DST_RGB */:
      return "GL_BLEND_DST_RGB (0x80C8)";
    case 0x80C9 /* GL_BLEND_SRC_RGB */:
      return "GL_BLEND_SRC_RGB (0x80C9)";
    case 0x80CA /* GL_BLEND_DST_ALPHA */:
      return "GL_BLEND_DST_ALPHA (0x80CA)";
    case 0x80CB /* GL_BLEND_SRC_ALPHA */:
      return "GL_BLEND_SRC_ALPHA (0x80CB)";
    case 0x8126 /* GL_POINT_SIZE_MIN */:
      return "GL_POINT_SIZE_MIN (0x8126)";
    case 0x8127 /* GL_POINT_SIZE_MAX */:
      return "GL_POINT_SIZE_MAX (0x8127)";
    case 0x8128 /* GL_POINT_FADE_THRESHOLD_SIZE */:
      return "GL_POINT_FADE_THRESHOLD_SIZE (0x8128)";
    case 0x8129 /* GL_POINT_DISTANCE_ATTENUATION */:
      return "GL_POINT_DISTANCE_ATTENUATION (0x8129)";
    case 0x8191 /* GL_GENERATE_MIPMAP */:
      return "GL_GENERATE_MIPMAP (0x8191)";
    case 0x8192 /* GL_GENERATE_MIPMAP_HINT */:
      return "GL_GENERATE_MIPMAP_HINT (0x8192)";
    case 0x81A5 /* GL_DEPTH_COMPONENT16 */:
      return "GL_DEPTH_COMPONENT16 (0x81A5)";
    case 0x81A6 /* GL_DEPTH_COMPONENT24 */:
      return "GL_DEPTH_COMPONENT24 (0x81A6)";
    case 0x81A7 /* GL_DEPTH_COMPONENT32 */:
      return "GL_DEPTH_COMPONENT32 (0x81A7)";
    case 0x8370 /* GL_MIRRORED_REPEAT */:
      return "GL_MIRRORED_REPEAT (0x8370)";
    case 0x8450 /* GL_FOG_COORDINATE_SOURCE */:
      return "GL_FOG_COORDINATE_SOURCE (0x8450)";
    case 0x8451 /* GL_FOG_COORDINATE */:
      return "GL_FOG_COORDINATE (0x8451)";
    case 0x8452 /* GL_FRAGMENT_DEPTH */:
      return "GL_FRAGMENT_DEPTH (0x8452)";
    case 0x8453 /* GL_CURRENT_FOG_COORDINATE */:
      return "GL_CURRENT_FOG_COORDINATE (0x8453)";
    case 0x8454 /* GL_FOG_COORDINATE_ARRAY_TYPE */:
      return "GL_FOG_COORDINATE_ARRAY_TYPE (0x8454)";
    case 0x8455 /* GL_FOG_COORDINATE_ARRAY_STRIDE */:
      return "GL_FOG_COORDINATE_ARRAY_STRIDE (0x8455)";
    case 0x8456 /* GL_FOG_COORDINATE_ARRAY_POINTER */:
      return "GL_FOG_COORDINATE_ARRAY_POINTER (0x8456)";
    case 0x8457 /* GL_FOG_COORDINATE_ARRAY */:
      return "GL_FOG_COORDINATE_ARRAY (0x8457)";
    case 0x8458 /* GL_COLOR_SUM */:
      return "GL_COLOR_SUM (0x8458)";
    case 0x8459 /* GL_CURRENT_SECONDARY_COLOR */:
      return "GL_CURRENT_SECONDARY_COLOR (0x8459)";
    case 0x845A /* GL_SECONDARY_COLOR_ARRAY_SIZE */:
      return "GL_SECONDARY_COLOR_ARRAY_SIZE (0x845A)";
    case 0x845B /* GL_SECONDARY_COLOR_ARRAY_TYPE */:
      return "GL_SECONDARY_COLOR_ARRAY_TYPE (0x845B)";
    case 0x845C /* GL_SECONDARY_COLOR_ARRAY_STRIDE */:
      return "GL_SECONDARY_COLOR_ARRAY_STRIDE (0x845C)";
    case 0x845D /* GL_SECONDARY_COLOR_ARRAY_POINTER */:
      return "GL_SECONDARY_COLOR_ARRAY_POINTER (0x845D)";
    case 0x845E /* GL_SECONDARY_COLOR_ARRAY */:
      return "GL_SECONDARY_COLOR_ARRAY (0x845E)";
    case 0x84FD /* GL_MAX_TEXTURE_LOD_BIAS */:
      return "GL_MAX_TEXTURE_LOD_BIAS (0x84FD)";
    case 0x8500 /* GL_TEXTURE_FILTER_CONTROL */:
      return "GL_TEXTURE_FILTER_CONTROL (0x8500)";
    case 0x8501 /* GL_TEXTURE_LOD_BIAS */:
      return "GL_TEXTURE_LOD_BIAS (0x8501)";
    case 0x8507 /* GL_INCR_WRAP */:
      return "GL_INCR_WRAP (0x8507)";
    case 0x8508 /* GL_DECR_WRAP */:
      return "GL_DECR_WRAP (0x8508)";
    case 0x884A /* GL_TEXTURE_DEPTH_SIZE */:
      return "GL_TEXTURE_DEPTH_SIZE (0x884A)";
    case 0x884B /* GL_DEPTH_TEXTURE_MODE */:
      return "GL_DEPTH_TEXTURE_MODE (0x884B)";
    case 0x884C /* GL_TEXTURE_COMPARE_MODE */:
      return "GL_TEXTURE_COMPARE_MODE (0x884C)";
    case 0x884D /* GL_TEXTURE_COMPARE_FUNC */:
      return "GL_TEXTURE_COMPARE_FUNC (0x884D)";
    case 0x884E /* GL_COMPARE_R_TO_TEXTURE */:
      return "GL_COMPARE_R_TO_TEXTURE (0x884E)";

      /* ----------------------------- GL_VERSION_1_5
       * ---------------------------- */

    case 0x8764 /* GL_BUFFER_SIZE */:
      return "GL_BUFFER_SIZE (0x8764)";
    case 0x8765 /* GL_BUFFER_USAGE */:
      return "GL_BUFFER_USAGE (0x8765)";
    case 0x8864 /* GL_QUERY_COUNTER_BITS */:
      return "GL_QUERY_COUNTER_BITS (0x8864)";
    case 0x8865 /* GL_CURRENT_QUERY */:
      return "GL_CURRENT_QUERY (0x8865)";
    case 0x8866 /* GL_QUERY_RESULT */:
      return "GL_QUERY_RESULT (0x8866)";
    case 0x8867 /* GL_QUERY_RESULT_AVAILABLE */:
      return "GL_QUERY_RESULT_AVAILABLE (0x8867)";
    case 0x8892 /* GL_ARRAY_BUFFER */:
      return "GL_ARRAY_BUFFER (0x8892)";
    case 0x8893 /* GL_ELEMENT_ARRAY_BUFFER */:
      return "GL_ELEMENT_ARRAY_BUFFER (0x8893)";
    case 0x8894 /* GL_ARRAY_BUFFER_BINDING */:
      return "GL_ARRAY_BUFFER_BINDING (0x8894)";
    case 0x8895 /* GL_ELEMENT_ARRAY_BUFFER_BINDING */:
      return "GL_ELEMENT_ARRAY_BUFFER_BINDING (0x8895)";
    case 0x8896 /* GL_VERTEX_ARRAY_BUFFER_BINDING */:
      return "GL_VERTEX_ARRAY_BUFFER_BINDING (0x8896)";
    case 0x8897 /* GL_NORMAL_ARRAY_BUFFER_BINDING */:
      return "GL_NORMAL_ARRAY_BUFFER_BINDING (0x8897)";
    case 0x8898 /* GL_COLOR_ARRAY_BUFFER_BINDING */:
      return "GL_COLOR_ARRAY_BUFFER_BINDING (0x8898)";
    case 0x8899 /* GL_INDEX_ARRAY_BUFFER_BINDING */:
      return "GL_INDEX_ARRAY_BUFFER_BINDING (0x8899)";
    case 0x889A /* GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING */:
      return "GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING (0x889A)";
    case 0x889B /* GL_EDGE_FLAG_ARRAY_BUFFER_BINDING */:
      return "GL_EDGE_FLAG_ARRAY_BUFFER_BINDING (0x889B)";
    case 0x889C /* GL_SECONDARY_COLOR_ARRAY_BUFFER_BINDING */:
      return "GL_SECONDARY_COLOR_ARRAY_BUFFER_BINDING (0x889C)";
    case 0x889D /* GL_FOG_COORDINATE_ARRAY_BUFFER_BINDING */:
      return "GL_FOG_COORDINATE_ARRAY_BUFFER_BINDING (0x889D)";
    case 0x889E /* GL_WEIGHT_ARRAY_BUFFER_BINDING */:
      return "GL_WEIGHT_ARRAY_BUFFER_BINDING (0x889E)";
    case 0x889F /* GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING */:
      return "GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING (0x889F)";
    case 0x88B8 /* GL_READ_ONLY */:
      return "GL_READ_ONLY (0x88B8)";
    case 0x88B9 /* GL_WRITE_ONLY */:
      return "GL_WRITE_ONLY (0x88B9)";
    case 0x88BA /* GL_READ_WRITE */:
      return "GL_READ_WRITE (0x88BA)";
    case 0x88BB /* GL_BUFFER_ACCESS */:
      return "GL_BUFFER_ACCESS (0x88BB)";
    case 0x88BC /* GL_BUFFER_MAPPED */:
      return "GL_BUFFER_MAPPED (0x88BC)";
    case 0x88BD /* GL_BUFFER_MAP_POINTER */:
      return "GL_BUFFER_MAP_POINTER (0x88BD)";
    case 0x88E0 /* GL_STREAM_DRAW */:
      return "GL_STREAM_DRAW (0x88E0)";
    case 0x88E1 /* GL_STREAM_READ */:
      return "GL_STREAM_READ (0x88E1)";
    case 0x88E2 /* GL_STREAM_COPY */:
      return "GL_STREAM_COPY (0x88E2)";
    case 0x88E4 /* GL_STATIC_DRAW */:
      return "GL_STATIC_DRAW (0x88E4)";
    case 0x88E5 /* GL_STATIC_READ */:
      return "GL_STATIC_READ (0x88E5)";
    case 0x88E6 /* GL_STATIC_COPY */:
      return "GL_STATIC_COPY (0x88E6)";
    case 0x88E8 /* GL_DYNAMIC_DRAW */:
      return "GL_DYNAMIC_DRAW (0x88E8)";
    case 0x88E9 /* GL_DYNAMIC_READ */:
      return "GL_DYNAMIC_READ (0x88E9)";
    case 0x88EA /* GL_DYNAMIC_COPY */:
      return "GL_DYNAMIC_COPY (0x88EA)";
    case 0x8914 /* GL_SAMPLES_PASSED */:
      return "GL_SAMPLES_PASSED (0x8914)";

      /* ----------------------------- GL_VERSION_2_0
       * ---------------------------- */

    case 0x8622 /* GL_VERTEX_ATTRIB_ARRAY_ENABLED */:
      return "GL_VERTEX_ATTRIB_ARRAY_ENABLED (0x8622)";
    case 0x8623 /* GL_VERTEX_ATTRIB_ARRAY_SIZE */:
      return "GL_VERTEX_ATTRIB_ARRAY_SIZE (0x8623)";
    case 0x8624 /* GL_VERTEX_ATTRIB_ARRAY_STRIDE */:
      return "GL_VERTEX_ATTRIB_ARRAY_STRIDE (0x8624)";
    case 0x8625 /* GL_VERTEX_ATTRIB_ARRAY_TYPE */:
      return "GL_VERTEX_ATTRIB_ARRAY_TYPE (0x8625)";
    case 0x8626 /* GL_CURRENT_VERTEX_ATTRIB */:
      return "GL_CURRENT_VERTEX_ATTRIB (0x8626)";
    // case 0x8642 /* GL_VERTEX_PROGRAM_POINT_SIZE */: return
    // "GL_VERTEX_PROGRAM_POINT_SIZE (0x8642)";
    case 0x8643 /* GL_VERTEX_PROGRAM_TWO_SIDE */:
      return "GL_VERTEX_PROGRAM_TWO_SIDE (0x8643)";
    case 0x8645 /* GL_VERTEX_ATTRIB_ARRAY_POINTER */:
      return "GL_VERTEX_ATTRIB_ARRAY_POINTER (0x8645)";
    case 0x8800 /* GL_STENCIL_BACK_FUNC */:
      return "GL_STENCIL_BACK_FUNC (0x8800)";
    case 0x8801 /* GL_STENCIL_BACK_FAIL */:
      return "GL_STENCIL_BACK_FAIL (0x8801)";
    case 0x8802 /* GL_STENCIL_BACK_PASS_DEPTH_FAIL */:
      return "GL_STENCIL_BACK_PASS_DEPTH_FAIL (0x8802)";
    case 0x8803 /* GL_STENCIL_BACK_PASS_DEPTH_PASS */:
      return "GL_STENCIL_BACK_PASS_DEPTH_PASS (0x8803)";
    case 0x8824 /* GL_MAX_DRAW_BUFFERS */:
      return "GL_MAX_DRAW_BUFFERS (0x8824)";
    case 0x8825 /* GL_DRAW_BUFFER0 */:
      return "GL_DRAW_BUFFER0 (0x8825)";
    case 0x8826 /* GL_DRAW_BUFFER1 */:
      return "GL_DRAW_BUFFER1 (0x8826)";
    case 0x8827 /* GL_DRAW_BUFFER2 */:
      return "GL_DRAW_BUFFER2 (0x8827)";
    case 0x8828 /* GL_DRAW_BUFFER3 */:
      return "GL_DRAW_BUFFER3 (0x8828)";
    case 0x8829 /* GL_DRAW_BUFFER4 */:
      return "GL_DRAW_BUFFER4 (0x8829)";
    case 0x882A /* GL_DRAW_BUFFER5 */:
      return "GL_DRAW_BUFFER5 (0x882A)";
    case 0x882B /* GL_DRAW_BUFFER6 */:
      return "GL_DRAW_BUFFER6 (0x882B)";
    case 0x882C /* GL_DRAW_BUFFER7 */:
      return "GL_DRAW_BUFFER7 (0x882C)";
    case 0x882D /* GL_DRAW_BUFFER8 */:
      return "GL_DRAW_BUFFER8 (0x882D)";
    case 0x882E /* GL_DRAW_BUFFER9 */:
      return "GL_DRAW_BUFFER9 (0x882E)";
    case 0x882F /* GL_DRAW_BUFFER10 */:
      return "GL_DRAW_BUFFER10 (0x882F)";
    case 0x8830 /* GL_DRAW_BUFFER11 */:
      return "GL_DRAW_BUFFER11 (0x8830)";
    case 0x8831 /* GL_DRAW_BUFFER12 */:
      return "GL_DRAW_BUFFER12 (0x8831)";
    case 0x8832 /* GL_DRAW_BUFFER13 */:
      return "GL_DRAW_BUFFER13 (0x8832)";
    case 0x8833 /* GL_DRAW_BUFFER14 */:
      return "GL_DRAW_BUFFER14 (0x8833)";
    case 0x8834 /* GL_DRAW_BUFFER15 */:
      return "GL_DRAW_BUFFER15 (0x8834)";
    case 0x883D /* GL_BLEND_EQUATION_ALPHA */:
      return "GL_BLEND_EQUATION_ALPHA (0x883D)";
    case 0x8861 /* GL_POINT_SPRITE */:
      return "GL_POINT_SPRITE (0x8861)";
    case 0x8862 /* GL_COORD_REPLACE */:
      return "GL_COORD_REPLACE (0x8862)";
    case 0x8869 /* GL_MAX_VERTEX_ATTRIBS */:
      return "GL_MAX_VERTEX_ATTRIBS (0x8869)";
    case 0x886A /* GL_VERTEX_ATTRIB_ARRAY_NORMALIZED */:
      return "GL_VERTEX_ATTRIB_ARRAY_NORMALIZED (0x886A)";
    case 0x8871 /* GL_MAX_TEXTURE_COORDS */:
      return "GL_MAX_TEXTURE_COORDS (0x8871)";
    case 0x8872 /* GL_MAX_TEXTURE_IMAGE_UNITS */:
      return "GL_MAX_TEXTURE_IMAGE_UNITS (0x8872)";
    case 0x8B30 /* GL_FRAGMENT_SHADER */:
      return "GL_FRAGMENT_SHADER (0x8B30)";
    case 0x8B31 /* GL_VERTEX_SHADER */:
      return "GL_VERTEX_SHADER (0x8B31)";
    case 0x8B49 /* GL_MAX_FRAGMENT_UNIFORM_COMPONENTS */:
      return "GL_MAX_FRAGMENT_UNIFORM_COMPONENTS (0x8B49)";
    case 0x8B4A /* GL_MAX_VERTEX_UNIFORM_COMPONENTS */:
      return "GL_MAX_VERTEX_UNIFORM_COMPONENTS (0x8B4A)";
    case 0x8B4B /* GL_MAX_VARYING_FLOATS */:
      return "GL_MAX_VARYING_FLOATS (0x8B4B)";
    case 0x8B4C /* GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS */:
      return "GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS (0x8B4C)";
    case 0x8B4D /* GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS */:
      return "GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS (0x8B4D)";
    case 0x8B4F /* GL_SHADER_TYPE */:
      return "GL_SHADER_TYPE (0x8B4F)";
    case 0x8B50 /* GL_FLOAT_VEC2 */:
      return "GL_FLOAT_VEC2 (0x8B50)";
    case 0x8B51 /* GL_FLOAT_VEC3 */:
      return "GL_FLOAT_VEC3 (0x8B51)";
    case 0x8B52 /* GL_FLOAT_VEC4 */:
      return "GL_FLOAT_VEC4 (0x8B52)";
    case 0x8B53 /* GL_INT_VEC2 */:
      return "GL_INT_VEC2 (0x8B53)";
    case 0x8B54 /* GL_INT_VEC3 */:
      return "GL_INT_VEC3 (0x8B54)";
    case 0x8B55 /* GL_INT_VEC4 */:
      return "GL_INT_VEC4 (0x8B55)";
    case 0x8B56 /* GL_BOOL */:
      return "GL_BOOL (0x8B56)";
    case 0x8B57 /* GL_BOOL_VEC2 */:
      return "GL_BOOL_VEC2 (0x8B57)";
    case 0x8B58 /* GL_BOOL_VEC3 */:
      return "GL_BOOL_VEC3 (0x8B58)";
    case 0x8B59 /* GL_BOOL_VEC4 */:
      return "GL_BOOL_VEC4 (0x8B59)";
    case 0x8B5A /* GL_FLOAT_MAT2 */:
      return "GL_FLOAT_MAT2 (0x8B5A)";
    case 0x8B5B /* GL_FLOAT_MAT3 */:
      return "GL_FLOAT_MAT3 (0x8B5B)";
    case 0x8B5C /* GL_FLOAT_MAT4 */:
      return "GL_FLOAT_MAT4 (0x8B5C)";
    case 0x8B5D /* GL_SAMPLER_1D */:
      return "GL_SAMPLER_1D (0x8B5D)";
    case 0x8B5E /* GL_SAMPLER_2D */:
      return "GL_SAMPLER_2D (0x8B5E)";
    case 0x8B5F /* GL_SAMPLER_3D */:
      return "GL_SAMPLER_3D (0x8B5F)";
    case 0x8B60 /* GL_SAMPLER_CUBE */:
      return "GL_SAMPLER_CUBE (0x8B60)";
    case 0x8B61 /* GL_SAMPLER_1D_SHADOW */:
      return "GL_SAMPLER_1D_SHADOW (0x8B61)";
    case 0x8B62 /* GL_SAMPLER_2D_SHADOW */:
      return "GL_SAMPLER_2D_SHADOW (0x8B62)";
    case 0x8B80 /* GL_DELETE_STATUS */:
      return "GL_DELETE_STATUS (0x8B80)";
    case 0x8B81 /* GL_COMPILE_STATUS */:
      return "GL_COMPILE_STATUS (0x8B81)";
    case 0x8B82 /* GL_LINK_STATUS */:
      return "GL_LINK_STATUS (0x8B82)";
    case 0x8B83 /* GL_VALIDATE_STATUS */:
      return "GL_VALIDATE_STATUS (0x8B83)";
    case 0x8B84 /* GL_INFO_LOG_LENGTH */:
      return "GL_INFO_LOG_LENGTH (0x8B84)";
    case 0x8B85 /* GL_ATTACHED_SHADERS */:
      return "GL_ATTACHED_SHADERS (0x8B85)";
    case 0x8B86 /* GL_ACTIVE_UNIFORMS */:
      return "GL_ACTIVE_UNIFORMS (0x8B86)";
    case 0x8B87 /* GL_ACTIVE_UNIFORM_MAX_LENGTH */:
      return "GL_ACTIVE_UNIFORM_MAX_LENGTH (0x8B87)";
    case 0x8B88 /* GL_SHADER_SOURCE_LENGTH */:
      return "GL_SHADER_SOURCE_LENGTH (0x8B88)";
    case 0x8B89 /* GL_ACTIVE_ATTRIBUTES */:
      return "GL_ACTIVE_ATTRIBUTES (0x8B89)";
    case 0x8B8A /* GL_ACTIVE_ATTRIBUTE_MAX_LENGTH */:
      return "GL_ACTIVE_ATTRIBUTE_MAX_LENGTH (0x8B8A)";
    case 0x8B8B /* GL_FRAGMENT_SHADER_DERIVATIVE_HINT */:
      return "GL_FRAGMENT_SHADER_DERIVATIVE_HINT (0x8B8B)";
    case 0x8B8C /* GL_SHADING_LANGUAGE_VERSION */:
      return "GL_SHADING_LANGUAGE_VERSION (0x8B8C)";
    case 0x8B8D /* GL_CURRENT_PROGRAM */:
      return "GL_CURRENT_PROGRAM (0x8B8D)";
    case 0x8CA0 /* GL_POINT_SPRITE_COORD_ORIGIN */:
      return "GL_POINT_SPRITE_COORD_ORIGIN (0x8CA0)";
    case 0x8CA1 /* GL_LOWER_LEFT */:
      return "GL_LOWER_LEFT (0x8CA1)";
    case 0x8CA2 /* GL_UPPER_LEFT */:
      return "GL_UPPER_LEFT (0x8CA2)";
    case 0x8CA3 /* GL_STENCIL_BACK_REF */:
      return "GL_STENCIL_BACK_REF (0x8CA3)";
    case 0x8CA4 /* GL_STENCIL_BACK_VALUE_MASK */:
      return "GL_STENCIL_BACK_VALUE_MASK (0x8CA4)";
    case 0x8CA5 /* GL_STENCIL_BACK_WRITEMASK */:
      return "GL_STENCIL_BACK_WRITEMASK (0x8CA5)";

      /* ----------------------------- GL_VERSION_2_1
       * ---------------------------- */

    case 0x845F /* GL_CURRENT_RASTER_SECONDARY_COLOR */:
      return "GL_CURRENT_RASTER_SECONDARY_COLOR (0x845F)";
    case 0x88EB /* GL_PIXEL_PACK_BUFFER */:
      return "GL_PIXEL_PACK_BUFFER (0x88EB)";
    case 0x88EC /* GL_PIXEL_UNPACK_BUFFER */:
      return "GL_PIXEL_UNPACK_BUFFER (0x88EC)";
    case 0x88ED /* GL_PIXEL_PACK_BUFFER_BINDING */:
      return "GL_PIXEL_PACK_BUFFER_BINDING (0x88ED)";
    case 0x88EF /* GL_PIXEL_UNPACK_BUFFER_BINDING */:
      return "GL_PIXEL_UNPACK_BUFFER_BINDING (0x88EF)";
    case 0x8B65 /* GL_FLOAT_MAT2x3 */:
      return "GL_FLOAT_MAT2x3 (0x8B65)";
    case 0x8B66 /* GL_FLOAT_MAT2x4 */:
      return "GL_FLOAT_MAT2x4 (0x8B66)";
    case 0x8B67 /* GL_FLOAT_MAT3x2 */:
      return "GL_FLOAT_MAT3x2 (0x8B67)";
    case 0x8B68 /* GL_FLOAT_MAT3x4 */:
      return "GL_FLOAT_MAT3x4 (0x8B68)";
    case 0x8B69 /* GL_FLOAT_MAT4x2 */:
      return "GL_FLOAT_MAT4x2 (0x8B69)";
    case 0x8B6A /* GL_FLOAT_MAT4x3 */:
      return "GL_FLOAT_MAT4x3 (0x8B6A)";
    case 0x8C40 /* GL_SRGB */:
      return "GL_SRGB (0x8C40)";
    case 0x8C41 /* GL_SRGB8 */:
      return "GL_SRGB8 (0x8C41)";
    case 0x8C42 /* GL_SRGB_ALPHA */:
      return "GL_SRGB_ALPHA (0x8C42)";
    case 0x8C43 /* GL_SRGB8_ALPHA8 */:
      return "GL_SRGB8_ALPHA8 (0x8C43)";
    case 0x8C44 /* GL_SLUMINANCE_ALPHA */:
      return "GL_SLUMINANCE_ALPHA (0x8C44)";
    case 0x8C45 /* GL_SLUMINANCE8_ALPHA8 */:
      return "GL_SLUMINANCE8_ALPHA8 (0x8C45)";
    case 0x8C46 /* GL_SLUMINANCE */:
      return "GL_SLUMINANCE (0x8C46)";
    case 0x8C47 /* GL_SLUMINANCE8 */:
      return "GL_SLUMINANCE8 (0x8C47)";
    case 0x8C48 /* GL_COMPRESSED_SRGB */:
      return "GL_COMPRESSED_SRGB (0x8C48)";
    case 0x8C49 /* GL_COMPRESSED_SRGB_ALPHA */:
      return "GL_COMPRESSED_SRGB_ALPHA (0x8C49)";
    case 0x8C4A /* GL_COMPRESSED_SLUMINANCE */:
      return "GL_COMPRESSED_SLUMINANCE (0x8C4A)";
    case 0x8C4B /* GL_COMPRESSED_SLUMINANCE_ALPHA */:
      return "GL_COMPRESSED_SLUMINANCE_ALPHA (0x8C4B)";

    /* ----------------------------- GL_VERSION_3_0 ----------------------------
     */

    // case 0x0001 /* GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT */: return
    // "GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT (0x0001)";
    case 0x821B /* GL_MAJOR_VERSION */:
      return "GL_MAJOR_VERSION (0x821B)";
    case 0x821C /* GL_MINOR_VERSION */:
      return "GL_MINOR_VERSION (0x821C)";
    case 0x821D /* GL_NUM_EXTENSIONS */:
      return "GL_NUM_EXTENSIONS (0x821D)";
    case 0x821E /* GL_CONTEXT_FLAGS */:
      return "GL_CONTEXT_FLAGS (0x821E)";
    case 0x8223 /* GL_DEPTH_BUFFER */:
      return "GL_DEPTH_BUFFER (0x8223)";
    case 0x8224 /* GL_STENCIL_BUFFER */:
      return "GL_STENCIL_BUFFER (0x8224)";
    case 0x8225 /* GL_COMPRESSED_RED */:
      return "GL_COMPRESSED_RED (0x8225)";
    case 0x8226 /* GL_COMPRESSED_RG */:
      return "GL_COMPRESSED_RG (0x8226)";
    case 0x8814 /* GL_RGBA32F */:
      return "GL_RGBA32F (0x8814)";
    // case 0x8814 /* GL_RGBA_FLOAT32_APPLE */: return "GL_RGBA_FLOAT32_APPLE
    // (0x8814)"; case 0x8814 /* GL_RGBA32F_ARB */: return "GL_RGBA32F_ARB
    // (0x8814)";
    case 0x8815 /* GL_RGB32F */:
      return "GL_RGB32F (0x8815)";
    case 0x881A /* GL_RGBA16F */:
      return "GL_RGBA16F (0x881A)";
    case 0x881B /* GL_RGB16F */:
      return "GL_RGB16F (0x881B)";
    case 0x88FD /* GL_VERTEX_ATTRIB_ARRAY_INTEGER */:
      return "GL_VERTEX_ATTRIB_ARRAY_INTEGER (0x88FD)";
    case 0x88FF /* GL_MAX_ARRAY_TEXTURE_LAYERS */:
      return "GL_MAX_ARRAY_TEXTURE_LAYERS (0x88FF)";
    case 0x8904 /* GL_MIN_PROGRAM_TEXEL_OFFSET */:
      return "GL_MIN_PROGRAM_TEXEL_OFFSET (0x8904)";
    case 0x8905 /* GL_MAX_PROGRAM_TEXEL_OFFSET */:
      return "GL_MAX_PROGRAM_TEXEL_OFFSET (0x8905)";
    case 0x891A /* GL_CLAMP_VERTEX_COLOR */:
      return "GL_CLAMP_VERTEX_COLOR (0x891A)";
    case 0x891B /* GL_CLAMP_FRAGMENT_COLOR */:
      return "GL_CLAMP_FRAGMENT_COLOR (0x891B)";
    case 0x891C /* GL_CLAMP_READ_COLOR */:
      return "GL_CLAMP_READ_COLOR (0x891C)";
    case 0x891D /* GL_FIXED_ONLY */:
      return "GL_FIXED_ONLY (0x891D)";
    case 0x8C10 /* GL_TEXTURE_RED_TYPE */:
      return "GL_TEXTURE_RED_TYPE (0x8C10)";
    case 0x8C11 /* GL_TEXTURE_GREEN_TYPE */:
      return "GL_TEXTURE_GREEN_TYPE (0x8C11)";
    case 0x8C12 /* GL_TEXTURE_BLUE_TYPE */:
      return "GL_TEXTURE_BLUE_TYPE (0x8C12)";
    case 0x8C13 /* GL_TEXTURE_ALPHA_TYPE */:
      return "GL_TEXTURE_ALPHA_TYPE (0x8C13)";
    case 0x8C14 /* GL_TEXTURE_LUMINANCE_TYPE */:
      return "GL_TEXTURE_LUMINANCE_TYPE (0x8C14)";
    case 0x8C15 /* GL_TEXTURE_INTENSITY_TYPE */:
      return "GL_TEXTURE_INTENSITY_TYPE (0x8C15)";
    case 0x8C16 /* GL_TEXTURE_DEPTH_TYPE */:
      return "GL_TEXTURE_DEPTH_TYPE (0x8C16)";
    case 0x8C18 /* GL_TEXTURE_1D_ARRAY */:
      return "GL_TEXTURE_1D_ARRAY (0x8C18)";
    case 0x8C19 /* GL_PROXY_TEXTURE_1D_ARRAY */:
      return "GL_PROXY_TEXTURE_1D_ARRAY (0x8C19)";
    case 0x8C1A /* GL_TEXTURE_2D_ARRAY */:
      return "GL_TEXTURE_2D_ARRAY (0x8C1A)";
    case 0x8C1B /* GL_PROXY_TEXTURE_2D_ARRAY */:
      return "GL_PROXY_TEXTURE_2D_ARRAY (0x8C1B)";
    case 0x8C1C /* GL_TEXTURE_BINDING_1D_ARRAY */:
      return "GL_TEXTURE_BINDING_1D_ARRAY (0x8C1C)";
    case 0x8C1D /* GL_TEXTURE_BINDING_2D_ARRAY */:
      return "GL_TEXTURE_BINDING_2D_ARRAY (0x8C1D)";
    case 0x8C3A /* GL_R11F_G11F_B10F */:
      return "GL_R11F_G11F_B10F (0x8C3A)";
    case 0x8C3B /* GL_UNSIGNED_INT_10F_11F_11F_REV */:
      return "GL_UNSIGNED_INT_10F_11F_11F_REV (0x8C3B)";
    case 0x8C3D /* GL_RGB9_E5 */:
      return "GL_RGB9_E5 (0x8C3D)";
    case 0x8C3E /* GL_UNSIGNED_INT_5_9_9_9_REV */:
      return "GL_UNSIGNED_INT_5_9_9_9_REV (0x8C3E)";
    case 0x8C3F /* GL_TEXTURE_SHARED_SIZE */:
      return "GL_TEXTURE_SHARED_SIZE (0x8C3F)";
    case 0x8C76 /* GL_TRANSFORM_FEEDBACK_VARYING_MAX_LENGTH */:
      return "GL_TRANSFORM_FEEDBACK_VARYING_MAX_LENGTH (0x8C76)";
    case 0x8C7F /* GL_TRANSFORM_FEEDBACK_BUFFER_MODE */:
      return "GL_TRANSFORM_FEEDBACK_BUFFER_MODE (0x8C7F)";
    case 0x8C80 /* GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS */:
      return "GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS (0x8C80)";
    case 0x8C83 /* GL_TRANSFORM_FEEDBACK_VARYINGS */:
      return "GL_TRANSFORM_FEEDBACK_VARYINGS (0x8C83)";
    case 0x8C84 /* GL_TRANSFORM_FEEDBACK_BUFFER_START */:
      return "GL_TRANSFORM_FEEDBACK_BUFFER_START (0x8C84)";
    case 0x8C85 /* GL_TRANSFORM_FEEDBACK_BUFFER_SIZE */:
      return "GL_TRANSFORM_FEEDBACK_BUFFER_SIZE (0x8C85)";
    case 0x8C87 /* GL_PRIMITIVES_GENERATED */:
      return "GL_PRIMITIVES_GENERATED (0x8C87)";
    case 0x8C88 /* GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN */:
      return "GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN (0x8C88)";
    case 0x8C89 /* GL_RASTERIZER_DISCARD */:
      return "GL_RASTERIZER_DISCARD (0x8C89)";
    case 0x8C8A /* GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS */:
      return "GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS (0x8C8A)";
    case 0x8C8B /* GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS */:
      return "GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS (0x8C8B)";
    case 0x8C8C /* GL_INTERLEAVED_ATTRIBS */:
      return "GL_INTERLEAVED_ATTRIBS (0x8C8C)";
    case 0x8C8D /* GL_SEPARATE_ATTRIBS */:
      return "GL_SEPARATE_ATTRIBS (0x8C8D)";
    case 0x8C8E /* GL_TRANSFORM_FEEDBACK_BUFFER */:
      return "GL_TRANSFORM_FEEDBACK_BUFFER (0x8C8E)";
    case 0x8C8F /* GL_TRANSFORM_FEEDBACK_BUFFER_BINDING */:
      return "GL_TRANSFORM_FEEDBACK_BUFFER_BINDING (0x8C8F)";
    case 0x8D70 /* GL_RGBA32UI */:
      return "GL_RGBA32UI (0x8D70)";
    case 0x8D71 /* GL_RGB32UI */:
      return "GL_RGB32UI (0x8D71)";
    case 0x8D76 /* GL_RGBA16UI */:
      return "GL_RGBA16UI (0x8D76)";
    case 0x8D77 /* GL_RGB16UI */:
      return "GL_RGB16UI (0x8D77)";
    case 0x8D7C /* GL_RGBA8UI */:
      return "GL_RGBA8UI (0x8D7C)";
    case 0x8D7D /* GL_RGB8UI */:
      return "GL_RGB8UI (0x8D7D)";
    case 0x8D82 /* GL_RGBA32I */:
      return "GL_RGBA32I (0x8D82)";
    case 0x8D83 /* GL_RGB32I */:
      return "GL_RGB32I (0x8D83)";
    case 0x8D88 /* GL_RGBA16I */:
      return "GL_RGBA16I (0x8D88)";
    case 0x8D89 /* GL_RGB16I */:
      return "GL_RGB16I (0x8D89)";
    case 0x8D8E /* GL_RGBA8I */:
      return "GL_RGBA8I (0x8D8E)";
    case 0x8D8F /* GL_RGB8I */:
      return "GL_RGB8I (0x8D8F)";
    case 0x8D94 /* GL_RED_INTEGER */:
      return "GL_RED_INTEGER (0x8D94)";
    case 0x8D95 /* GL_GREEN_INTEGER */:
      return "GL_GREEN_INTEGER (0x8D95)";
    case 0x8D96 /* GL_BLUE_INTEGER */:
      return "GL_BLUE_INTEGER (0x8D96)";
    case 0x8D97 /* GL_ALPHA_INTEGER */:
      return "GL_ALPHA_INTEGER (0x8D97)";
    case 0x8D98 /* GL_RGB_INTEGER */:
      return "GL_RGB_INTEGER (0x8D98)";
    case 0x8D99 /* GL_RGBA_INTEGER */:
      return "GL_RGBA_INTEGER (0x8D99)";
    case 0x8D9A /* GL_BGR_INTEGER */:
      return "GL_BGR_INTEGER (0x8D9A)";
    case 0x8D9B /* GL_BGRA_INTEGER */:
      return "GL_BGRA_INTEGER (0x8D9B)";
    case 0x8DC0 /* GL_SAMPLER_1D_ARRAY */:
      return "GL_SAMPLER_1D_ARRAY (0x8DC0)";
    case 0x8DC1 /* GL_SAMPLER_2D_ARRAY */:
      return "GL_SAMPLER_2D_ARRAY (0x8DC1)";
    case 0x8DC3 /* GL_SAMPLER_1D_ARRAY_SHADOW */:
      return "GL_SAMPLER_1D_ARRAY_SHADOW (0x8DC3)";
    case 0x8DC4 /* GL_SAMPLER_2D_ARRAY_SHADOW */:
      return "GL_SAMPLER_2D_ARRAY_SHADOW (0x8DC4)";
    case 0x8DC5 /* GL_SAMPLER_CUBE_SHADOW */:
      return "GL_SAMPLER_CUBE_SHADOW (0x8DC5)";
    case 0x8DC6 /* GL_UNSIGNED_INT_VEC2 */:
      return "GL_UNSIGNED_INT_VEC2 (0x8DC6)";
    case 0x8DC7 /* GL_UNSIGNED_INT_VEC3 */:
      return "GL_UNSIGNED_INT_VEC3 (0x8DC7)";
    case 0x8DC8 /* GL_UNSIGNED_INT_VEC4 */:
      return "GL_UNSIGNED_INT_VEC4 (0x8DC8)";
    case 0x8DC9 /* GL_INT_SAMPLER_1D */:
      return "GL_INT_SAMPLER_1D (0x8DC9)";
    case 0x8DCA /* GL_INT_SAMPLER_2D */:
      return "GL_INT_SAMPLER_2D (0x8DCA)";
    case 0x8DCB /* GL_INT_SAMPLER_3D */:
      return "GL_INT_SAMPLER_3D (0x8DCB)";
    case 0x8DCC /* GL_INT_SAMPLER_CUBE */:
      return "GL_INT_SAMPLER_CUBE (0x8DCC)";
    case 0x8DCE /* GL_INT_SAMPLER_1D_ARRAY */:
      return "GL_INT_SAMPLER_1D_ARRAY (0x8DCE)";
    case 0x8DCF /* GL_INT_SAMPLER_2D_ARRAY */:
      return "GL_INT_SAMPLER_2D_ARRAY (0x8DCF)";
    case 0x8DD1 /* GL_UNSIGNED_INT_SAMPLER_1D */:
      return "GL_UNSIGNED_INT_SAMPLER_1D (0x8DD1)";
    case 0x8DD2 /* GL_UNSIGNED_INT_SAMPLER_2D */:
      return "GL_UNSIGNED_INT_SAMPLER_2D (0x8DD2)";
    case 0x8DD3 /* GL_UNSIGNED_INT_SAMPLER_3D */:
      return "GL_UNSIGNED_INT_SAMPLER_3D (0x8DD3)";
    case 0x8DD4 /* GL_UNSIGNED_INT_SAMPLER_CUBE */:
      return "GL_UNSIGNED_INT_SAMPLER_CUBE (0x8DD4)";
    case 0x8DD6 /* GL_UNSIGNED_INT_SAMPLER_1D_ARRAY */:
      return "GL_UNSIGNED_INT_SAMPLER_1D_ARRAY (0x8DD6)";
    case 0x8DD7 /* GL_UNSIGNED_INT_SAMPLER_2D_ARRAY */:
      return "GL_UNSIGNED_INT_SAMPLER_2D_ARRAY (0x8DD7)";
    case 0x8E13 /* GL_QUERY_WAIT */:
      return "GL_QUERY_WAIT (0x8E13)";
    case 0x8E14 /* GL_QUERY_NO_WAIT */:
      return "GL_QUERY_NO_WAIT (0x8E14)";
    case 0x8E15 /* GL_QUERY_BY_REGION_WAIT */:
      return "GL_QUERY_BY_REGION_WAIT (0x8E15)";
    case 0x8E16 /* GL_QUERY_BY_REGION_NO_WAIT */:
      return "GL_QUERY_BY_REGION_NO_WAIT (0x8E16)";

      /* ----------------------------- GL_VERSION_3_1
       * ---------------------------- */

    case 0x84F5 /* GL_TEXTURE_RECTANGLE */:
      return "GL_TEXTURE_RECTANGLE (0x84F5)";
    case 0x84F6 /* GL_TEXTURE_BINDING_RECTANGLE */:
      return "GL_TEXTURE_BINDING_RECTANGLE (0x84F6)";
    case 0x84F7 /* GL_PROXY_TEXTURE_RECTANGLE */:
      return "GL_PROXY_TEXTURE_RECTANGLE (0x84F7)";
    case 0x84F8 /* GL_MAX_RECTANGLE_TEXTURE_SIZE */:
      return "GL_MAX_RECTANGLE_TEXTURE_SIZE (0x84F8)";
    case 0x8B63 /* GL_SAMPLER_2D_RECT */:
      return "GL_SAMPLER_2D_RECT (0x8B63)";
    case 0x8B64 /* GL_SAMPLER_2D_RECT_SHADOW */:
      return "GL_SAMPLER_2D_RECT_SHADOW (0x8B64)";
    case 0x8C2A /* GL_TEXTURE_BUFFER */:
      return "GL_TEXTURE_BUFFER (0x8C2A)";
    case 0x8C2B /* GL_MAX_TEXTURE_BUFFER_SIZE */:
      return "GL_MAX_TEXTURE_BUFFER_SIZE (0x8C2B)";
    case 0x8C2C /* GL_TEXTURE_BINDING_BUFFER */:
      return "GL_TEXTURE_BINDING_BUFFER (0x8C2C)";
    case 0x8C2D /* GL_TEXTURE_BUFFER_DATA_STORE_BINDING */:
      return "GL_TEXTURE_BUFFER_DATA_STORE_BINDING (0x8C2D)";
    case 0x8C2E /* GL_TEXTURE_BUFFER_FORMAT */:
      return "GL_TEXTURE_BUFFER_FORMAT (0x8C2E)";
    case 0x8DC2 /* GL_SAMPLER_BUFFER */:
      return "GL_SAMPLER_BUFFER (0x8DC2)";
    case 0x8DCD /* GL_INT_SAMPLER_2D_RECT */:
      return "GL_INT_SAMPLER_2D_RECT (0x8DCD)";
    case 0x8DD0 /* GL_INT_SAMPLER_BUFFER */:
      return "GL_INT_SAMPLER_BUFFER (0x8DD0)";
    case 0x8DD5 /* GL_UNSIGNED_INT_SAMPLER_2D_RECT */:
      return "GL_UNSIGNED_INT_SAMPLER_2D_RECT (0x8DD5)";
    case 0x8DD8 /* GL_UNSIGNED_INT_SAMPLER_BUFFER */:
      return "GL_UNSIGNED_INT_SAMPLER_BUFFER (0x8DD8)";
    case 0x8F90 /* GL_RED_SNORM */:
      return "GL_RED_SNORM (0x8F90)";
    case 0x8F91 /* GL_RG_SNORM */:
      return "GL_RG_SNORM (0x8F91)";
    case 0x8F92 /* GL_RGB_SNORM */:
      return "GL_RGB_SNORM (0x8F92)";
    case 0x8F93 /* GL_RGBA_SNORM */:
      return "GL_RGBA_SNORM (0x8F93)";
    case 0x8F94 /* GL_R8_SNORM */:
      return "GL_R8_SNORM (0x8F94)";
    case 0x8F95 /* GL_RG8_SNORM */:
      return "GL_RG8_SNORM (0x8F95)";
    case 0x8F96 /* GL_RGB8_SNORM */:
      return "GL_RGB8_SNORM (0x8F96)";
    case 0x8F97 /* GL_RGBA8_SNORM */:
      return "GL_RGBA8_SNORM (0x8F97)";
    case 0x8F98 /* GL_R16_SNORM */:
      return "GL_R16_SNORM (0x8F98)";
    case 0x8F99 /* GL_RG16_SNORM */:
      return "GL_RG16_SNORM (0x8F99)";
    case 0x8F9A /* GL_RGB16_SNORM */:
      return "GL_RGB16_SNORM (0x8F9A)";
    case 0x8F9B /* GL_RGBA16_SNORM */:
      return "GL_RGBA16_SNORM (0x8F9B)";
    case 0x8F9C /* GL_SIGNED_NORMALIZED */:
      return "GL_SIGNED_NORMALIZED (0x8F9C)";
    case 0x8F9D /* GL_PRIMITIVE_RESTART */:
      return "GL_PRIMITIVE_RESTART (0x8F9D)";
    case 0x8F9E /* GL_PRIMITIVE_RESTART_INDEX */:
      return "GL_PRIMITIVE_RESTART_INDEX (0x8F9E)";
    case 0x911F /* GL_BUFFER_ACCESS_FLAGS */:
      return "GL_BUFFER_ACCESS_FLAGS (0x911F)";
    case 0x9120 /* GL_BUFFER_MAP_LENGTH */:
      return "GL_BUFFER_MAP_LENGTH (0x9120)";
    case 0x9121 /* GL_BUFFER_MAP_OFFSET */:
      return "GL_BUFFER_MAP_OFFSET (0x9121)";

    /* ----------------------------- GL_VERSION_3_2 ----------------------------
     */

    // case 0x00000001 /* GL_CONTEXT_CORE_PROFILE_BIT */: return
    // "GL_CONTEXT_CORE_PROFILE_BIT (0x00000001)"; case 0x00000002 /*
    // GL_CONTEXT_COMPATIBILITY_PROFILE_BIT */: return
    // "GL_CONTEXT_COMPATIBILITY_PROFILE_BIT (0x00000002)";
    case 0x000A /* GL_LINES_ADJACENCY */:
      return "GL_LINES_ADJACENCY (0x000A)";
    case 0x000B /* GL_LINE_STRIP_ADJACENCY */:
      return "GL_LINE_STRIP_ADJACENCY (0x000B)";
    case 0x000C /* GL_TRIANGLES_ADJACENCY */:
      return "GL_TRIANGLES_ADJACENCY (0x000C)";
    case 0x000D /* GL_TRIANGLE_STRIP_ADJACENCY */:
      return "GL_TRIANGLE_STRIP_ADJACENCY (0x000D)";
    case 0x8642 /* GL_PROGRAM_POINT_SIZE */:
      return "GL_PROGRAM_POINT_SIZE (0x8642)";
    // case 0x8642 /* GL_PROGRAM_POINT_SIZE_ARB */: return
    // "GL_PROGRAM_POINT_SIZE_ARB (0x8642)";
    case 0x8916 /* GL_GEOMETRY_VERTICES_OUT */:
      return "GL_GEOMETRY_VERTICES_OUT (0x8916)";
    case 0x8917 /* GL_GEOMETRY_INPUT_TYPE */:
      return "GL_GEOMETRY_INPUT_TYPE (0x8917)";
    case 0x8918 /* GL_GEOMETRY_OUTPUT_TYPE */:
      return "GL_GEOMETRY_OUTPUT_TYPE (0x8918)";
    case 0x8C29 /* GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS */:
      return "GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS (0x8C29)";
    case 0x8DA7 /* GL_FRAMEBUFFER_ATTACHMENT_LAYERED */:
      return "GL_FRAMEBUFFER_ATTACHMENT_LAYERED (0x8DA7)";
    case 0x8DA8 /* GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS */:
      return "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS (0x8DA8)";
    case 0x8DD9 /* GL_GEOMETRY_SHADER */:
      return "GL_GEOMETRY_SHADER (0x8DD9)";
    case 0x8DDF /* GL_MAX_GEOMETRY_UNIFORM_COMPONENTS */:
      return "GL_MAX_GEOMETRY_UNIFORM_COMPONENTS (0x8DDF)";
    case 0x8DE0 /* GL_MAX_GEOMETRY_OUTPUT_VERTICES */:
      return "GL_MAX_GEOMETRY_OUTPUT_VERTICES (0x8DE0)";
    case 0x8DE1 /* GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS */:
      return "GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS (0x8DE1)";
    case 0x9122 /* GL_MAX_VERTEX_OUTPUT_COMPONENTS */:
      return "GL_MAX_VERTEX_OUTPUT_COMPONENTS (0x9122)";
    case 0x9123 /* GL_MAX_GEOMETRY_INPUT_COMPONENTS */:
      return "GL_MAX_GEOMETRY_INPUT_COMPONENTS (0x9123)";
    case 0x9124 /* GL_MAX_GEOMETRY_OUTPUT_COMPONENTS */:
      return "GL_MAX_GEOMETRY_OUTPUT_COMPONENTS (0x9124)";
    case 0x9125 /* GL_MAX_FRAGMENT_INPUT_COMPONENTS */:
      return "GL_MAX_FRAGMENT_INPUT_COMPONENTS (0x9125)";
    case 0x9126 /* GL_CONTEXT_PROFILE_MASK */:
      return "GL_CONTEXT_PROFILE_MASK (0x9126)";

      /* ----------------------------- GL_VERSION_3_3
       * ---------------------------- */

    case 0x88FE /* GL_VERTEX_ATTRIB_ARRAY_DIVISOR */:
      return "GL_VERTEX_ATTRIB_ARRAY_DIVISOR (0x88FE)";
    case 0x8E42 /* GL_TEXTURE_SWIZZLE_R */:
      return "GL_TEXTURE_SWIZZLE_R (0x8E42)";
    case 0x8E43 /* GL_TEXTURE_SWIZZLE_G */:
      return "GL_TEXTURE_SWIZZLE_G (0x8E43)";
    case 0x8E44 /* GL_TEXTURE_SWIZZLE_B */:
      return "GL_TEXTURE_SWIZZLE_B (0x8E44)";
    case 0x8E45 /* GL_TEXTURE_SWIZZLE_A */:
      return "GL_TEXTURE_SWIZZLE_A (0x8E45)";
    case 0x8E46 /* GL_TEXTURE_SWIZZLE_RGBA */:
      return "GL_TEXTURE_SWIZZLE_RGBA (0x8E46)";
    case 0x906F /* GL_RGB10_A2UI */:
      return "GL_RGB10_A2UI (0x906F)";

      /* ----------------------------- GL_VERSION_4_0
       * ---------------------------- */

    case 0x887F /* GL_GEOMETRY_SHADER_INVOCATIONS */:
      return "GL_GEOMETRY_SHADER_INVOCATIONS (0x887F)";
    case 0x8C36 /* GL_SAMPLE_SHADING */:
      return "GL_SAMPLE_SHADING (0x8C36)";
    case 0x8C37 /* GL_MIN_SAMPLE_SHADING_VALUE */:
      return "GL_MIN_SAMPLE_SHADING_VALUE (0x8C37)";
    case 0x8E5A /* GL_MAX_GEOMETRY_SHADER_INVOCATIONS */:
      return "GL_MAX_GEOMETRY_SHADER_INVOCATIONS (0x8E5A)";
    case 0x8E5B /* GL_MIN_FRAGMENT_INTERPOLATION_OFFSET */:
      return "GL_MIN_FRAGMENT_INTERPOLATION_OFFSET (0x8E5B)";
    case 0x8E5C /* GL_MAX_FRAGMENT_INTERPOLATION_OFFSET */:
      return "GL_MAX_FRAGMENT_INTERPOLATION_OFFSET (0x8E5C)";
    case 0x8E5D /* GL_FRAGMENT_INTERPOLATION_OFFSET_BITS */:
      return "GL_FRAGMENT_INTERPOLATION_OFFSET_BITS (0x8E5D)";
    case 0x8E5E /* GL_MIN_PROGRAM_TEXTURE_GATHER_OFFSET */:
      return "GL_MIN_PROGRAM_TEXTURE_GATHER_OFFSET (0x8E5E)";
    case 0x8E5F /* GL_MAX_PROGRAM_TEXTURE_GATHER_OFFSET */:
      return "GL_MAX_PROGRAM_TEXTURE_GATHER_OFFSET (0x8E5F)";
    case 0x8F9F /* GL_MAX_PROGRAM_TEXTURE_GATHER_COMPONENTS */:
      return "GL_MAX_PROGRAM_TEXTURE_GATHER_COMPONENTS (0x8F9F)";
    case 0x9009 /* GL_TEXTURE_CUBE_MAP_ARRAY */:
      return "GL_TEXTURE_CUBE_MAP_ARRAY (0x9009)";
    case 0x900A /* GL_TEXTURE_BINDING_CUBE_MAP_ARRAY */:
      return "GL_TEXTURE_BINDING_CUBE_MAP_ARRAY (0x900A)";
    case 0x900B /* GL_PROXY_TEXTURE_CUBE_MAP_ARRAY */:
      return "GL_PROXY_TEXTURE_CUBE_MAP_ARRAY (0x900B)";
    case 0x900C /* GL_SAMPLER_CUBE_MAP_ARRAY */:
      return "GL_SAMPLER_CUBE_MAP_ARRAY (0x900C)";
    case 0x900D /* GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW */:
      return "GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW (0x900D)";
    case 0x900E /* GL_INT_SAMPLER_CUBE_MAP_ARRAY */:
      return "GL_INT_SAMPLER_CUBE_MAP_ARRAY (0x900E)";
    case 0x900F /* GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY */:
      return "GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY (0x900F)";

      /* ----------------------------- GL_VERSION_4_2
       * ---------------------------- */

    case 0x8E8C /* GL_COMPRESSED_RGBA_BPTC_UNORM */:
      return "GL_COMPRESSED_RGBA_BPTC_UNORM (0x8E8C)";
    case 0x8E8D /* GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM */:
      return "GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM (0x8E8D)";
    case 0x8E8E /* GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT */:
      return "GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT (0x8E8E)";
    case 0x8E8F /* GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT */:
      return "GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT (0x8E8F)";

      /* ----------------------------- GL_VERSION_4_3
       * ---------------------------- */

    case 0x82E9 /* GL_NUM_SHADING_LANGUAGE_VERSIONS */:
      return "GL_NUM_SHADING_LANGUAGE_VERSIONS (0x82E9)";
    case 0x874E /* GL_VERTEX_ATTRIB_ARRAY_LONG */:
      return "GL_VERTEX_ATTRIB_ARRAY_LONG (0x874E)";

      /* -------------------------- GL_3DFX_multisample
       * -------------------------- */

    case 0x86B2 /* GL_MULTISAMPLE_3DFX */:
      return "GL_MULTISAMPLE_3DFX (0x86B2)";
    case 0x86B3 /* GL_SAMPLE_BUFFERS_3DFX */:
      return "GL_SAMPLE_BUFFERS_3DFX (0x86B3)";
    case 0x86B4 /* GL_SAMPLES_3DFX */:
      return "GL_SAMPLES_3DFX (0x86B4)";
      // case 0x20000000 /* GL_MULTISAMPLE_BIT_3DFX */: return
      // "GL_MULTISAMPLE_BIT_3DFX (0x20000000)";

      /* ---------------------------- GL_3DFX_tbuffer
       * ---------------------------- */

      /* -------------------- GL_3DFX_texture_compression_FXT1
       * ------------------- */

    case 0x86B0 /* GL_COMPRESSED_RGB_FXT1_3DFX */:
      return "GL_COMPRESSED_RGB_FXT1_3DFX (0x86B0)";
    case 0x86B1 /* GL_COMPRESSED_RGBA_FXT1_3DFX */:
      return "GL_COMPRESSED_RGBA_FXT1_3DFX (0x86B1)";

      /* ----------------------- GL_AMD_blend_minmax_factor
       * ---------------------- */

    case 0x901C /* GL_FACTOR_MIN_AMD */:
      return "GL_FACTOR_MIN_AMD (0x901C)";
    case 0x901D /* GL_FACTOR_MAX_AMD */:
      return "GL_FACTOR_MAX_AMD (0x901D)";

      /* ----------------------- GL_AMD_conservative_depth
       * ----------------------- */

      /* -------------------------- GL_AMD_debug_output
       * -------------------------- */

    case 0x9143 /* GL_MAX_DEBUG_MESSAGE_LENGTH_AMD */:
      return "GL_MAX_DEBUG_MESSAGE_LENGTH_AMD (0x9143)";
    case 0x9144 /* GL_MAX_DEBUG_LOGGED_MESSAGES_AMD */:
      return "GL_MAX_DEBUG_LOGGED_MESSAGES_AMD (0x9144)";
    case 0x9145 /* GL_DEBUG_LOGGED_MESSAGES_AMD */:
      return "GL_DEBUG_LOGGED_MESSAGES_AMD (0x9145)";
    case 0x9146 /* GL_DEBUG_SEVERITY_HIGH_AMD */:
      return "GL_DEBUG_SEVERITY_HIGH_AMD (0x9146)";
    case 0x9147 /* GL_DEBUG_SEVERITY_MEDIUM_AMD */:
      return "GL_DEBUG_SEVERITY_MEDIUM_AMD (0x9147)";
    case 0x9148 /* GL_DEBUG_SEVERITY_LOW_AMD */:
      return "GL_DEBUG_SEVERITY_LOW_AMD (0x9148)";
    case 0x9149 /* GL_DEBUG_CATEGORY_API_ERROR_AMD */:
      return "GL_DEBUG_CATEGORY_API_ERROR_AMD (0x9149)";
    case 0x914A /* GL_DEBUG_CATEGORY_WINDOW_SYSTEM_AMD */:
      return "GL_DEBUG_CATEGORY_WINDOW_SYSTEM_AMD (0x914A)";
    case 0x914B /* GL_DEBUG_CATEGORY_DEPRECATION_AMD */:
      return "GL_DEBUG_CATEGORY_DEPRECATION_AMD (0x914B)";
    case 0x914C /* GL_DEBUG_CATEGORY_UNDEFINED_BEHAVIOR_AMD */:
      return "GL_DEBUG_CATEGORY_UNDEFINED_BEHAVIOR_AMD (0x914C)";
    case 0x914D /* GL_DEBUG_CATEGORY_PERFORMANCE_AMD */:
      return "GL_DEBUG_CATEGORY_PERFORMANCE_AMD (0x914D)";
    case 0x914E /* GL_DEBUG_CATEGORY_SHADER_COMPILER_AMD */:
      return "GL_DEBUG_CATEGORY_SHADER_COMPILER_AMD (0x914E)";
    case 0x914F /* GL_DEBUG_CATEGORY_APPLICATION_AMD */:
      return "GL_DEBUG_CATEGORY_APPLICATION_AMD (0x914F)";
    case 0x9150 /* GL_DEBUG_CATEGORY_OTHER_AMD */:
      return "GL_DEBUG_CATEGORY_OTHER_AMD (0x9150)";

      /* ---------------------- GL_AMD_depth_clamp_separate
       * ---------------------- */

    case 0x901E /* GL_DEPTH_CLAMP_NEAR_AMD */:
      return "GL_DEPTH_CLAMP_NEAR_AMD (0x901E)";
    case 0x901F /* GL_DEPTH_CLAMP_FAR_AMD */:
      return "GL_DEPTH_CLAMP_FAR_AMD (0x901F)";

      /* ----------------------- GL_AMD_draw_buffers_blend
       * ----------------------- */

      /* ----------------------- GL_AMD_multi_draw_indirect
       * ---------------------- */

      /* ------------------------- GL_AMD_name_gen_delete
       * ------------------------ */

    case 0x9151 /* GL_DATA_BUFFER_AMD */:
      return "GL_DATA_BUFFER_AMD (0x9151)";
    case 0x9152 /* GL_PERFORMANCE_MONITOR_AMD */:
      return "GL_PERFORMANCE_MONITOR_AMD (0x9152)";
    case 0x9153 /* GL_QUERY_OBJECT_AMD */:
      return "GL_QUERY_OBJECT_AMD (0x9153)";
    case 0x9154 /* GL_VERTEX_ARRAY_OBJECT_AMD */:
      return "GL_VERTEX_ARRAY_OBJECT_AMD (0x9154)";
    case 0x9155 /* GL_SAMPLER_OBJECT_AMD */:
      return "GL_SAMPLER_OBJECT_AMD (0x9155)";

      /* ----------------------- GL_AMD_performance_monitor
       * ---------------------- */

    case 0x8BC0 /* GL_COUNTER_TYPE_AMD */:
      return "GL_COUNTER_TYPE_AMD (0x8BC0)";
    case 0x8BC1 /* GL_COUNTER_RANGE_AMD */:
      return "GL_COUNTER_RANGE_AMD (0x8BC1)";
    case 0x8BC2 /* GL_UNSIGNED_INT64_AMD */:
      return "GL_UNSIGNED_INT64_AMD (0x8BC2)";
    case 0x8BC3 /* GL_PERCENTAGE_AMD */:
      return "GL_PERCENTAGE_AMD (0x8BC3)";
    case 0x8BC4 /* GL_PERFMON_RESULT_AVAILABLE_AMD */:
      return "GL_PERFMON_RESULT_AVAILABLE_AMD (0x8BC4)";
    case 0x8BC5 /* GL_PERFMON_RESULT_SIZE_AMD */:
      return "GL_PERFMON_RESULT_SIZE_AMD (0x8BC5)";
    case 0x8BC6 /* GL_PERFMON_RESULT_AMD */:
      return "GL_PERFMON_RESULT_AMD (0x8BC6)"; /* --------------------------
                                                  GL_AMD_pinned_memory
                                                  ------------------------- */

    case 0x9160 /* GL_EXTERNAL_VIRTUAL_MEMORY_BUFFER_AMD */:
      return "GL_EXTERNAL_VIRTUAL_MEMORY_BUFFER_AMD (0x9160)";

      /* ----------------------- GL_AMD_query_buffer_object
       * ---------------------- */

    case 0x9192 /* GL_QUERY_BUFFER_AMD */:
      return "GL_QUERY_BUFFER_AMD (0x9192)";
    case 0x9193 /* GL_QUERY_BUFFER_BINDING_AMD */:
      return "GL_QUERY_BUFFER_BINDING_AMD (0x9193)";
    case 0x9194 /* GL_QUERY_RESULT_NO_WAIT_AMD */:
      return "GL_QUERY_RESULT_NO_WAIT_AMD (0x9194)";

      /* ------------------------ GL_AMD_sample_positions
       * ------------------------ */

    case 0x883F /* GL_SUBSAMPLE_DISTANCE_AMD */:
      return "GL_SUBSAMPLE_DISTANCE_AMD (0x883F)"; /* ------------------
                                                      GL_AMD_seamless_cubemap_per_texture
                                                      ------------------ */

    case 0x884F /* GL_TEXTURE_CUBE_MAP_SEAMLESS_ARB */:
      return "GL_TEXTURE_CUBE_MAP_SEAMLESS_ARB (0x884F)";

      /* ---------------------- GL_AMD_shader_stencil_export
       * --------------------- */

      /* ------------------- GL_AMD_stencil_operation_extended
       * ------------------- */

    case 0x874A /* GL_SET_AMD */:
      return "GL_SET_AMD (0x874A)";
    case 0x874B /* GL_REPLACE_VALUE_AMD */:
      return "GL_REPLACE_VALUE_AMD (0x874B)";
    case 0x874C /* GL_STENCIL_OP_VALUE_AMD */:
      return "GL_STENCIL_OP_VALUE_AMD (0x874C)";
    case 0x874D /* GL_STENCIL_BACK_OP_VALUE_AMD */:
      return "GL_STENCIL_BACK_OP_VALUE_AMD (0x874D)"; /* ------------------------
                                                         GL_AMD_texture_texture4
                                                         ------------------------
                                                       */

      /* --------------- GL_AMD_transform_feedback3_lines_triangles
       * -------------- */

      /* ----------------------- GL_AMD_vertex_shader_layer
       * ---------------------- */

      /* -------------------- GL_AMD_vertex_shader_tessellator
       * ------------------- */

    case 0x9001 /* GL_SAMPLER_BUFFER_AMD */:
      return "GL_SAMPLER_BUFFER_AMD (0x9001)";
    case 0x9002 /* GL_INT_SAMPLER_BUFFER_AMD */:
      return "GL_INT_SAMPLER_BUFFER_AMD (0x9002)";
    case 0x9003 /* GL_UNSIGNED_INT_SAMPLER_BUFFER_AMD */:
      return "GL_UNSIGNED_INT_SAMPLER_BUFFER_AMD (0x9003)";
    case 0x9004 /* GL_TESSELLATION_MODE_AMD */:
      return "GL_TESSELLATION_MODE_AMD (0x9004)";
    case 0x9005 /* GL_TESSELLATION_FACTOR_AMD */:
      return "GL_TESSELLATION_FACTOR_AMD (0x9005)";
    case 0x9006 /* GL_DISCRETE_AMD */:
      return "GL_DISCRETE_AMD (0x9006)";
    case 0x9007 /* GL_CONTINUOUS_AMD */:
      return "GL_CONTINUOUS_AMD (0x9007)";

      /* ------------------ GL_AMD_vertex_shader_viewport_index
       * ------------------ */

      /* ----------------------- GL_APPLE_aux_depth_stencil
       * ---------------------- */

    case 0x8A14 /* GL_AUX_DEPTH_STENCIL_APPLE */:
      return "GL_AUX_DEPTH_STENCIL_APPLE (0x8A14)";

      /* ------------------------ GL_APPLE_client_storage
       * ------------------------ */

    case 0x85B2 /* GL_UNPACK_CLIENT_STORAGE_APPLE */:
      return "GL_UNPACK_CLIENT_STORAGE_APPLE (0x85B2)";

      /* ------------------------- GL_APPLE_element_array
       * ------------------------ */

    case 0x8A0C /* GL_ELEMENT_ARRAY_APPLE */:
      return "GL_ELEMENT_ARRAY_APPLE (0x8A0C)";
    case 0x8A0D /* GL_ELEMENT_ARRAY_TYPE_APPLE */:
      return "GL_ELEMENT_ARRAY_TYPE_APPLE (0x8A0D)";
    case 0x8A0E /* GL_ELEMENT_ARRAY_POINTER_APPLE */:
      return "GL_ELEMENT_ARRAY_POINTER_APPLE (0x8A0E)";

      /* ----------------------------- GL_APPLE_fence
       * ---------------------------- */

    case 0x8A0A /* GL_DRAW_PIXELS_APPLE */:
      return "GL_DRAW_PIXELS_APPLE (0x8A0A)";
    case 0x8A0B /* GL_FENCE_APPLE */:
      return "GL_FENCE_APPLE (0x8A0B)";

    /* ------------------------- GL_APPLE_float_pixels -------------------------
     */

    // case 0x140B /* GL_HALF_APPLE */: return "GL_HALF_APPLE (0x140B)";
    // case 0x8815 /* GL_RGB_FLOAT32_APPLE */: return "GL_RGB_FLOAT32_APPLE
    // (0x8815)";
    case 0x8816 /* GL_ALPHA_FLOAT32_APPLE */:
      return "GL_ALPHA_FLOAT32_APPLE (0x8816)";
    case 0x8817 /* GL_INTENSITY_FLOAT32_APPLE */:
      return "GL_INTENSITY_FLOAT32_APPLE (0x8817)";
    case 0x8818 /* GL_LUMINANCE_FLOAT32_APPLE */:
      return "GL_LUMINANCE_FLOAT32_APPLE (0x8818)";
    case 0x8819 /* GL_LUMINANCE_ALPHA_FLOAT32_APPLE */:
      return "GL_LUMINANCE_ALPHA_FLOAT32_APPLE (0x8819)";
    // case 0x881A /* GL_RGBA_FLOAT16_APPLE */: return "GL_RGBA_FLOAT16_APPLE
    // (0x881A)"; case 0x881B /* GL_RGB_FLOAT16_APPLE */: return
    // "GL_RGB_FLOAT16_APPLE (0x881B)";
    case 0x881C /* GL_ALPHA_FLOAT16_APPLE */:
      return "GL_ALPHA_FLOAT16_APPLE (0x881C)";
    case 0x881D /* GL_INTENSITY_FLOAT16_APPLE */:
      return "GL_INTENSITY_FLOAT16_APPLE (0x881D)";
    case 0x881E /* GL_LUMINANCE_FLOAT16_APPLE */:
      return "GL_LUMINANCE_FLOAT16_APPLE (0x881E)";
    case 0x881F /* GL_LUMINANCE_ALPHA_FLOAT16_APPLE */:
      return "GL_LUMINANCE_ALPHA_FLOAT16_APPLE (0x881F)";
    case 0x8A0F /* GL_COLOR_FLOAT_APPLE */:
      return "GL_COLOR_FLOAT_APPLE (0x8A0F)";

      /* ---------------------- GL_APPLE_flush_buffer_range
       * ---------------------- */

    case 0x8A12 /* GL_BUFFER_SERIALIZED_MODIFY_APPLE */:
      return "GL_BUFFER_SERIALIZED_MODIFY_APPLE (0x8A12)";
    case 0x8A13 /* GL_BUFFER_FLUSHING_UNMAP_APPLE */:
      return "GL_BUFFER_FLUSHING_UNMAP_APPLE (0x8A13)";

      /* ----------------------- GL_APPLE_object_purgeable
       * ----------------------- */

    case 0x85B3 /* GL_BUFFER_OBJECT_APPLE */:
      return "GL_BUFFER_OBJECT_APPLE (0x85B3)";
    case 0x8A19 /* GL_RELEASED_APPLE */:
      return "GL_RELEASED_APPLE (0x8A19)";
    case 0x8A1A /* GL_VOLATILE_APPLE */:
      return "GL_VOLATILE_APPLE (0x8A1A)";
    case 0x8A1B /* GL_RETAINED_APPLE */:
      return "GL_RETAINED_APPLE (0x8A1B)";
    case 0x8A1C /* GL_UNDEFINED_APPLE */:
      return "GL_UNDEFINED_APPLE (0x8A1C)";
    case 0x8A1D /* GL_PURGEABLE_APPLE */:
      return "GL_PURGEABLE_APPLE (0x8A1D)";

      /* ------------------------- GL_APPLE_pixel_buffer
       * ------------------------- */

    case 0x8A10 /* GL_MIN_PBUFFER_VIEWPORT_DIMS_APPLE */:
      return "GL_MIN_PBUFFER_VIEWPORT_DIMS_APPLE (0x8A10)";

      /* ---------------------------- GL_APPLE_rgb_422
       * --------------------------- */

    case 0x85BA /* GL_UNSIGNED_SHORT_8_8_APPLE */:
      return "GL_UNSIGNED_SHORT_8_8_APPLE (0x85BA)";
    case 0x85BB /* GL_UNSIGNED_SHORT_8_8_REV_APPLE */:
      return "GL_UNSIGNED_SHORT_8_8_REV_APPLE (0x85BB)";
    case 0x8A1F /* GL_RGB_422_APPLE */:
      return "GL_RGB_422_APPLE (0x8A1F)";

      /* --------------------------- GL_APPLE_row_bytes
       * -------------------------- */

    case 0x8A15 /* GL_PACK_ROW_BYTES_APPLE */:
      return "GL_PACK_ROW_BYTES_APPLE (0x8A15)";
    case 0x8A16 /* GL_UNPACK_ROW_BYTES_APPLE */:
      return "GL_UNPACK_ROW_BYTES_APPLE (0x8A16)";

      /* ------------------------ GL_APPLE_specular_vector
       * ----------------------- */

    case 0x85B0 /* GL_LIGHT_MODEL_SPECULAR_VECTOR_APPLE */:
      return "GL_LIGHT_MODEL_SPECULAR_VECTOR_APPLE (0x85B0)";

      /* ------------------------- GL_APPLE_texture_range
       * ------------------------ */

    case 0x85B7 /* GL_TEXTURE_RANGE_LENGTH_APPLE */:
      return "GL_TEXTURE_RANGE_LENGTH_APPLE (0x85B7)";
    case 0x85B8 /* GL_TEXTURE_RANGE_POINTER_APPLE */:
      return "GL_TEXTURE_RANGE_POINTER_APPLE (0x85B8)";
    case 0x85BC /* GL_TEXTURE_STORAGE_HINT_APPLE */:
      return "GL_TEXTURE_STORAGE_HINT_APPLE (0x85BC)";
    case 0x85BD /* GL_STORAGE_PRIVATE_APPLE */:
      return "GL_STORAGE_PRIVATE_APPLE (0x85BD)";
      // case 0x85BE /* GL_STORAGE_CACHED_APPLE */: return
      // "GL_STORAGE_CACHED_APPLE (0x85BE)"; case 0x85BF /*
      // GL_STORAGE_SHARED_APPLE */: return "GL_STORAGE_SHARED_APPLE (0x85BF)";

      /* ------------------------ GL_APPLE_transform_hint
       * ------------------------ */

    case 0x85B1 /* GL_TRANSFORM_HINT_APPLE */:
      return "GL_TRANSFORM_HINT_APPLE (0x85B1)";

      /* ---------------------- GL_APPLE_vertex_array_object
       * --------------------- */

    case 0x85B5 /* GL_VERTEX_ARRAY_BINDING_APPLE */:
      return "GL_VERTEX_ARRAY_BINDING_APPLE (0x85B5)";

      /* ---------------------- GL_APPLE_vertex_array_range
       * ---------------------- */

    case 0x851D /* GL_VERTEX_ARRAY_RANGE_APPLE */:
      return "GL_VERTEX_ARRAY_RANGE_APPLE (0x851D)";
    case 0x851E /* GL_VERTEX_ARRAY_RANGE_LENGTH_APPLE */:
      return "GL_VERTEX_ARRAY_RANGE_LENGTH_APPLE (0x851E)";
    case 0x851F /* GL_VERTEX_ARRAY_STORAGE_HINT_APPLE */:
      return "GL_VERTEX_ARRAY_STORAGE_HINT_APPLE (0x851F)";
    case 0x8520 /* GL_MAX_VERTEX_ARRAY_RANGE_ELEMENT_APPLE */:
      return "GL_MAX_VERTEX_ARRAY_RANGE_ELEMENT_APPLE (0x8520)";
    case 0x8521 /* GL_VERTEX_ARRAY_RANGE_POINTER_APPLE */:
      return "GL_VERTEX_ARRAY_RANGE_POINTER_APPLE (0x8521)";
    case 0x85B4 /* GL_STORAGE_CLIENT_APPLE */:
      return "GL_STORAGE_CLIENT_APPLE (0x85B4)";
    case 0x85BE /* GL_STORAGE_CACHED_APPLE */:
      return "GL_STORAGE_CACHED_APPLE (0x85BE)";
    case 0x85BF /* GL_STORAGE_SHARED_APPLE */:
      return "GL_STORAGE_SHARED_APPLE (0x85BF)";

      /* ------------------- GL_APPLE_vertex_program_evaluators
       * ------------------ */

    case 0x8A00 /* GL_VERTEX_ATTRIB_MAP1_APPLE */:
      return "GL_VERTEX_ATTRIB_MAP1_APPLE (0x8A00)";
    case 0x8A01 /* GL_VERTEX_ATTRIB_MAP2_APPLE */:
      return "GL_VERTEX_ATTRIB_MAP2_APPLE (0x8A01)";
    case 0x8A02 /* GL_VERTEX_ATTRIB_MAP1_SIZE_APPLE */:
      return "GL_VERTEX_ATTRIB_MAP1_SIZE_APPLE (0x8A02)";
    case 0x8A03 /* GL_VERTEX_ATTRIB_MAP1_COEFF_APPLE */:
      return "GL_VERTEX_ATTRIB_MAP1_COEFF_APPLE (0x8A03)";
    case 0x8A04 /* GL_VERTEX_ATTRIB_MAP1_ORDER_APPLE */:
      return "GL_VERTEX_ATTRIB_MAP1_ORDER_APPLE (0x8A04)";
    case 0x8A05 /* GL_VERTEX_ATTRIB_MAP1_DOMAIN_APPLE */:
      return "GL_VERTEX_ATTRIB_MAP1_DOMAIN_APPLE (0x8A05)";
    case 0x8A06 /* GL_VERTEX_ATTRIB_MAP2_SIZE_APPLE */:
      return "GL_VERTEX_ATTRIB_MAP2_SIZE_APPLE (0x8A06)";
    case 0x8A07 /* GL_VERTEX_ATTRIB_MAP2_COEFF_APPLE */:
      return "GL_VERTEX_ATTRIB_MAP2_COEFF_APPLE (0x8A07)";
    case 0x8A08 /* GL_VERTEX_ATTRIB_MAP2_ORDER_APPLE */:
      return "GL_VERTEX_ATTRIB_MAP2_ORDER_APPLE (0x8A08)";
    case 0x8A09 /* GL_VERTEX_ATTRIB_MAP2_DOMAIN_APPLE */:
      return "GL_VERTEX_ATTRIB_MAP2_DOMAIN_APPLE (0x8A09)";

      /* --------------------------- GL_APPLE_ycbcr_422
       * -------------------------- */

    case 0x85B9 /* GL_YCBCR_422_APPLE */:
      return "GL_YCBCR_422_APPLE (0x85B9)";

      /* ------------------------ GL_ARB_ES2_compatibility
       * ----------------------- */

    case 0x140C /* GL_FIXED */:
      return "GL_FIXED (0x140C)";
    case 0x8B9A /* GL_IMPLEMENTATION_COLOR_READ_TYPE */:
      return "GL_IMPLEMENTATION_COLOR_READ_TYPE (0x8B9A)";
    case 0x8B9B /* GL_IMPLEMENTATION_COLOR_READ_FORMAT */:
      return "GL_IMPLEMENTATION_COLOR_READ_FORMAT (0x8B9B)";
    case 0x8D62 /* GL_RGB565 */:
      return "GL_RGB565 (0x8D62)";
    case 0x8DF0 /* GL_LOW_FLOAT */:
      return "GL_LOW_FLOAT (0x8DF0)";
    case 0x8DF1 /* GL_MEDIUM_FLOAT */:
      return "GL_MEDIUM_FLOAT (0x8DF1)";
    case 0x8DF2 /* GL_HIGH_FLOAT */:
      return "GL_HIGH_FLOAT (0x8DF2)";
    case 0x8DF3 /* GL_LOW_INT */:
      return "GL_LOW_INT (0x8DF3)";
    case 0x8DF4 /* GL_MEDIUM_INT */:
      return "GL_MEDIUM_INT (0x8DF4)";
    case 0x8DF5 /* GL_HIGH_INT */:
      return "GL_HIGH_INT (0x8DF5)";
    case 0x8DF8 /* GL_SHADER_BINARY_FORMATS */:
      return "GL_SHADER_BINARY_FORMATS (0x8DF8)";
    case 0x8DF9 /* GL_NUM_SHADER_BINARY_FORMATS */:
      return "GL_NUM_SHADER_BINARY_FORMATS (0x8DF9)";
    case 0x8DFA /* GL_SHADER_COMPILER */:
      return "GL_SHADER_COMPILER (0x8DFA)";
    case 0x8DFB /* GL_MAX_VERTEX_UNIFORM_VECTORS */:
      return "GL_MAX_VERTEX_UNIFORM_VECTORS (0x8DFB)";
    case 0x8DFC /* GL_MAX_VARYING_VECTORS */:
      return "GL_MAX_VARYING_VECTORS (0x8DFC)";
    case 0x8DFD /* GL_MAX_FRAGMENT_UNIFORM_VECTORS */:
      return "GL_MAX_FRAGMENT_UNIFORM_VECTORS (0x8DFD)";

      /* ------------------------ GL_ARB_ES3_compatibility
       * ----------------------- */

    case 0x8D69 /* GL_PRIMITIVE_RESTART_FIXED_INDEX */:
      return "GL_PRIMITIVE_RESTART_FIXED_INDEX (0x8D69)";
    case 0x8D6A /* GL_ANY_SAMPLES_PASSED_CONSERVATIVE */:
      return "GL_ANY_SAMPLES_PASSED_CONSERVATIVE (0x8D6A)";
    case 0x8D6B /* GL_MAX_ELEMENT_INDEX */:
      return "GL_MAX_ELEMENT_INDEX (0x8D6B)";
    case 0x9270 /* GL_COMPRESSED_R11_EAC */:
      return "GL_COMPRESSED_R11_EAC (0x9270)";
    case 0x9271 /* GL_COMPRESSED_SIGNED_R11_EAC */:
      return "GL_COMPRESSED_SIGNED_R11_EAC (0x9271)";
    case 0x9272 /* GL_COMPRESSED_RG11_EAC */:
      return "GL_COMPRESSED_RG11_EAC (0x9272)";
    case 0x9273 /* GL_COMPRESSED_SIGNED_RG11_EAC */:
      return "GL_COMPRESSED_SIGNED_RG11_EAC (0x9273)";
    case 0x9274 /* GL_COMPRESSED_RGB8_ETC2 */:
      return "GL_COMPRESSED_RGB8_ETC2 (0x9274)";
    case 0x9275 /* GL_COMPRESSED_SRGB8_ETC2 */:
      return "GL_COMPRESSED_SRGB8_ETC2 (0x9275)";
    case 0x9276 /* GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2 */:
      return "GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2 (0x9276)";
    case 0x9277 /* GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2 */:
      return "GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2 (0x9277)";
    case 0x9278 /* GL_COMPRESSED_RGBA8_ETC2_EAC */:
      return "GL_COMPRESSED_RGBA8_ETC2_EAC (0x9278)";
    case 0x9279 /* GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC */:
      return "GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC (0x9279)";

      /* ------------------------ GL_ARB_arrays_of_arrays
       * ------------------------ */

      /* -------------------------- GL_ARB_base_instance
       * ------------------------- */

      /* ----------------------- GL_ARB_blend_func_extended
       * ---------------------- */

    case 0x88F9 /* GL_SRC1_COLOR */:
      return "GL_SRC1_COLOR (0x88F9)";
    case 0x88FA /* GL_ONE_MINUS_SRC1_COLOR */:
      return "GL_ONE_MINUS_SRC1_COLOR (0x88FA)";
    case 0x88FB /* GL_ONE_MINUS_SRC1_ALPHA */:
      return "GL_ONE_MINUS_SRC1_ALPHA (0x88FB)";
    case 0x88FC /* GL_MAX_DUAL_SOURCE_DRAW_BUFFERS */:
      return "GL_MAX_DUAL_SOURCE_DRAW_BUFFERS (0x88FC)";

      /* ---------------------------- GL_ARB_cl_event
       * ---------------------------- */

    case 0x8240 /* GL_SYNC_CL_EVENT_ARB */:
      return "GL_SYNC_CL_EVENT_ARB (0x8240)";
    case 0x8241 /* GL_SYNC_CL_EVENT_COMPLETE_ARB */:
      return "GL_SYNC_CL_EVENT_COMPLETE_ARB (0x8241)";

      /* ----------------------- GL_ARB_clear_buffer_object
       * ---------------------- */

      /* ----------------------- GL_ARB_color_buffer_float
       * ----------------------- */

    case 0x8820 /* GL_RGBA_FLOAT_MODE_ARB */:
      return "GL_RGBA_FLOAT_MODE_ARB (0x8820)";
      // case 0x891A /* GL_CLAMP_VERTEX_COLOR_ARB */: return
      // "GL_CLAMP_VERTEX_COLOR_ARB (0x891A)"; case 0x891B /*
      // GL_CLAMP_FRAGMENT_COLOR_ARB */: return "GL_CLAMP_FRAGMENT_COLOR_ARB
      // (0x891B)"; case 0x891C /* GL_CLAMP_READ_COLOR_ARB */: return
      // "GL_CLAMP_READ_COLOR_ARB (0x891C)"; case 0x891D /* GL_FIXED_ONLY_ARB */:
      // return "GL_FIXED_ONLY_ARB (0x891D)";

      /* -------------------------- GL_ARB_compatibility
       * ------------------------- */

      /* ---------------- GL_ARB_compressed_texture_pixel_storage
       * ---------------- */

    case 0x9127 /* GL_UNPACK_COMPRESSED_BLOCK_WIDTH */:
      return "GL_UNPACK_COMPRESSED_BLOCK_WIDTH (0x9127)";
    case 0x9128 /* GL_UNPACK_COMPRESSED_BLOCK_HEIGHT */:
      return "GL_UNPACK_COMPRESSED_BLOCK_HEIGHT (0x9128)";
    case 0x9129 /* GL_UNPACK_COMPRESSED_BLOCK_DEPTH */:
      return "GL_UNPACK_COMPRESSED_BLOCK_DEPTH (0x9129)";
    case 0x912A /* GL_UNPACK_COMPRESSED_BLOCK_SIZE */:
      return "GL_UNPACK_COMPRESSED_BLOCK_SIZE (0x912A)";
    case 0x912B /* GL_PACK_COMPRESSED_BLOCK_WIDTH */:
      return "GL_PACK_COMPRESSED_BLOCK_WIDTH (0x912B)";
    case 0x912C /* GL_PACK_COMPRESSED_BLOCK_HEIGHT */:
      return "GL_PACK_COMPRESSED_BLOCK_HEIGHT (0x912C)";
    case 0x912D /* GL_PACK_COMPRESSED_BLOCK_DEPTH */:
      return "GL_PACK_COMPRESSED_BLOCK_DEPTH (0x912D)";
    case 0x912E /* GL_PACK_COMPRESSED_BLOCK_SIZE */:
      return "GL_PACK_COMPRESSED_BLOCK_SIZE (0x912E)";

    /* ------------------------- GL_ARB_compute_shader -------------------------
     */

    // case 0x00000020 /* GL_COMPUTE_SHADER_BIT */: return
    // "GL_COMPUTE_SHADER_BIT (0x00000020)";
    case 0x8262 /* GL_MAX_COMPUTE_SHARED_MEMORY_SIZE */:
      return "GL_MAX_COMPUTE_SHARED_MEMORY_SIZE (0x8262)";
    case 0x8263 /* GL_MAX_COMPUTE_UNIFORM_COMPONENTS */:
      return "GL_MAX_COMPUTE_UNIFORM_COMPONENTS (0x8263)";
    case 0x8264 /* GL_MAX_COMPUTE_ATOMIC_COUNTER_BUFFERS */:
      return "GL_MAX_COMPUTE_ATOMIC_COUNTER_BUFFERS (0x8264)";
    case 0x8265 /* GL_MAX_COMPUTE_ATOMIC_COUNTERS */:
      return "GL_MAX_COMPUTE_ATOMIC_COUNTERS (0x8265)";
    case 0x8266 /* GL_MAX_COMBINED_COMPUTE_UNIFORM_COMPONENTS */:
      return "GL_MAX_COMBINED_COMPUTE_UNIFORM_COMPONENTS (0x8266)";
    case 0x8267 /* GL_COMPUTE_WORK_GROUP_SIZE */:
      return "GL_COMPUTE_WORK_GROUP_SIZE (0x8267)";
    case 0x90EB /* GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS */:
      return "GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS (0x90EB)";
    case 0x90EC /* GL_UNIFORM_BLOCK_REFERENCED_BY_COMPUTE_SHADER */:
      return "GL_UNIFORM_BLOCK_REFERENCED_BY_COMPUTE_SHADER (0x90EC)";
    case 0x90ED /* GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_COMPUTE_SHADER */:
      return "GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_COMPUTE_SHADER (0x90ED)";
    case 0x90EE /* GL_DISPATCH_INDIRECT_BUFFER */:
      return "GL_DISPATCH_INDIRECT_BUFFER (0x90EE)";
    case 0x90EF /* GL_DISPATCH_INDIRECT_BUFFER_BINDING */:
      return "GL_DISPATCH_INDIRECT_BUFFER_BINDING (0x90EF)";
    case 0x91B9 /* GL_COMPUTE_SHADER */:
      return "GL_COMPUTE_SHADER (0x91B9)";
    case 0x91BB /* GL_MAX_COMPUTE_UNIFORM_BLOCKS */:
      return "GL_MAX_COMPUTE_UNIFORM_BLOCKS (0x91BB)";
    case 0x91BC /* GL_MAX_COMPUTE_TEXTURE_IMAGE_UNITS */:
      return "GL_MAX_COMPUTE_TEXTURE_IMAGE_UNITS (0x91BC)";
    case 0x91BD /* GL_MAX_COMPUTE_IMAGE_UNIFORMS */:
      return "GL_MAX_COMPUTE_IMAGE_UNIFORMS (0x91BD)";
    case 0x91BE /* GL_MAX_COMPUTE_WORK_GROUP_COUNT */:
      return "GL_MAX_COMPUTE_WORK_GROUP_COUNT (0x91BE)";
    case 0x91BF /* GL_MAX_COMPUTE_WORK_GROUP_SIZE */:
      return "GL_MAX_COMPUTE_WORK_GROUP_SIZE (0x91BF)";

      /* ----------------------- GL_ARB_conservative_depth
       * ----------------------- */

      /* --------------------------- GL_ARB_copy_buffer
       * -------------------------- */

    case 0x8F36 /* GL_COPY_READ_BUFFER */:
      return "GL_COPY_READ_BUFFER (0x8F36)";
    case 0x8F37 /* GL_COPY_WRITE_BUFFER */:
      return "GL_COPY_WRITE_BUFFER (0x8F37)"; /* ---------------------------
                                                 GL_ARB_copy_image
                                                 --------------------------- */

      /* -------------------------- GL_ARB_debug_output
       * -------------------------- */

    case 0x8242 /* GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB */:
      return "GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB (0x8242)";
    case 0x8243 /* GL_DEBUG_NEXT_LOGGED_MESSAGE_LENGTH_ARB */:
      return "GL_DEBUG_NEXT_LOGGED_MESSAGE_LENGTH_ARB (0x8243)";
    case 0x8244 /* GL_DEBUG_CALLBACK_FUNCTION_ARB */:
      return "GL_DEBUG_CALLBACK_FUNCTION_ARB (0x8244)";
    case 0x8245 /* GL_DEBUG_CALLBACK_USER_PARAM_ARB */:
      return "GL_DEBUG_CALLBACK_USER_PARAM_ARB (0x8245)";
    case 0x8246 /* GL_DEBUG_SOURCE_API_ARB */:
      return "GL_DEBUG_SOURCE_API_ARB (0x8246)";
    case 0x8247 /* GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB */:
      return "GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB (0x8247)";
    case 0x8248 /* GL_DEBUG_SOURCE_SHADER_COMPILER_ARB */:
      return "GL_DEBUG_SOURCE_SHADER_COMPILER_ARB (0x8248)";
    case 0x8249 /* GL_DEBUG_SOURCE_THIRD_PARTY_ARB */:
      return "GL_DEBUG_SOURCE_THIRD_PARTY_ARB (0x8249)";
    case 0x824A /* GL_DEBUG_SOURCE_APPLICATION_ARB */:
      return "GL_DEBUG_SOURCE_APPLICATION_ARB (0x824A)";
    case 0x824B /* GL_DEBUG_SOURCE_OTHER_ARB */:
      return "GL_DEBUG_SOURCE_OTHER_ARB (0x824B)";
    case 0x824C /* GL_DEBUG_TYPE_ERROR_ARB */:
      return "GL_DEBUG_TYPE_ERROR_ARB (0x824C)";
    case 0x824D /* GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB */:
      return "GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB (0x824D)";
    case 0x824E /* GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB */:
      return "GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB (0x824E)";
    case 0x824F /* GL_DEBUG_TYPE_PORTABILITY_ARB */:
      return "GL_DEBUG_TYPE_PORTABILITY_ARB (0x824F)";
    case 0x8250 /* GL_DEBUG_TYPE_PERFORMANCE_ARB */:
      return "GL_DEBUG_TYPE_PERFORMANCE_ARB (0x8250)";
    case 0x8251 /* GL_DEBUG_TYPE_OTHER_ARB */:
      return "GL_DEBUG_TYPE_OTHER_ARB (0x8251)";
      // case 0x9143 /* GL_MAX_DEBUG_MESSAGE_LENGTH_ARB */: return
      // "GL_MAX_DEBUG_MESSAGE_LENGTH_ARB (0x9143)"; case 0x9144 /*
      // GL_MAX_DEBUG_LOGGED_MESSAGES_ARB */: return
      // "GL_MAX_DEBUG_LOGGED_MESSAGES_ARB (0x9144)"; case 0x9145 /*
      // GL_DEBUG_LOGGED_MESSAGES_ARB */: return "GL_DEBUG_LOGGED_MESSAGES_ARB
      // (0x9145)"; case 0x9146 /* GL_DEBUG_SEVERITY_HIGH_ARB */: return
      // "GL_DEBUG_SEVERITY_HIGH_ARB (0x9146)"; case 0x9147 /*
      // GL_DEBUG_SEVERITY_MEDIUM_ARB */: return "GL_DEBUG_SEVERITY_MEDIUM_ARB
      // (0x9147)"; case 0x9148 /* GL_DEBUG_SEVERITY_LOW_ARB */: return
      // "GL_DEBUG_SEVERITY_LOW_ARB (0x9148)";

      /* ----------------------- GL_ARB_depth_buffer_float
       * ----------------------- */

    case 0x8CAC /* GL_DEPTH_COMPONENT32F */:
      return "GL_DEPTH_COMPONENT32F (0x8CAC)";
    case 0x8CAD /* GL_DEPTH32F_STENCIL8 */:
      return "GL_DEPTH32F_STENCIL8 (0x8CAD)";
    case 0x8DAD /* GL_FLOAT_32_UNSIGNED_INT_24_8_REV */:
      return "GL_FLOAT_32_UNSIGNED_INT_24_8_REV (0x8DAD)";

      /* --------------------------- GL_ARB_depth_clamp
       * -------------------------- */

    case 0x864F /* GL_DEPTH_CLAMP */:
      return "GL_DEPTH_CLAMP (0x864F)";

      /* -------------------------- GL_ARB_depth_texture
       * ------------------------- */

      // case 0x81A5 /* GL_DEPTH_COMPONENT16_ARB */: return
      // "GL_DEPTH_COMPONENT16_ARB (0x81A5)"; case 0x81A6 /*
      // GL_DEPTH_COMPONENT24_ARB */: return "GL_DEPTH_COMPONENT24_ARB (0x81A6)";
      // case 0x81A7 /* GL_DEPTH_COMPONENT32_ARB */: return
      // "GL_DEPTH_COMPONENT32_ARB (0x81A7)"; case 0x884A /*
      // GL_TEXTURE_DEPTH_SIZE_ARB */: return "GL_TEXTURE_DEPTH_SIZE_ARB
      // (0x884A)"; case 0x884B /* GL_DEPTH_TEXTURE_MODE_ARB */: return
      // "GL_DEPTH_TEXTURE_MODE_ARB (0x884B)";

      /* -------------------------- GL_ARB_draw_buffers
       * -------------------------- */

      // case 0x8824 /* GL_MAX_DRAW_BUFFERS_ARB */: return
      // "GL_MAX_DRAW_BUFFERS_ARB (0x8824)"; case 0x8825 /* GL_DRAW_BUFFER0_ARB
      // */: return "GL_DRAW_BUFFER0_ARB (0x8825)"; case 0x8826 /*
      // GL_DRAW_BUFFER1_ARB */: return "GL_DRAW_BUFFER1_ARB (0x8826)"; case
      // 0x8827 /* GL_DRAW_BUFFER2_ARB */: return "GL_DRAW_BUFFER2_ARB (0x8827)";
      // case 0x8828 /* GL_DRAW_BUFFER3_ARB */: return "GL_DRAW_BUFFER3_ARB
      // (0x8828)"; case 0x8829 /* GL_DRAW_BUFFER4_ARB */: return
      // "GL_DRAW_BUFFER4_ARB (0x8829)"; case 0x882A /* GL_DRAW_BUFFER5_ARB */:
      // return "GL_DRAW_BUFFER5_ARB (0x882A)"; case 0x882B /*
      // GL_DRAW_BUFFER6_ARB */: return "GL_DRAW_BUFFER6_ARB (0x882B)"; case
      // 0x882C /* GL_DRAW_BUFFER7_ARB */: return "GL_DRAW_BUFFER7_ARB (0x882C)";
      // case 0x882D /* GL_DRAW_BUFFER8_ARB */: return "GL_DRAW_BUFFER8_ARB
      // (0x882D)"; case 0x882E /* GL_DRAW_BUFFER9_ARB */: return
      // "GL_DRAW_BUFFER9_ARB (0x882E)"; case 0x882F /* GL_DRAW_BUFFER10_ARB */:
      // return "GL_DRAW_BUFFER10_ARB (0x882F)"; case 0x8830 /*
      // GL_DRAW_BUFFER11_ARB */: return "GL_DRAW_BUFFER11_ARB (0x8830)"; case
      // 0x8831 /* GL_DRAW_BUFFER12_ARB */: return "GL_DRAW_BUFFER12_ARB
      // (0x8831)"; case 0x8832 /* GL_DRAW_BUFFER13_ARB */: return
      // "GL_DRAW_BUFFER13_ARB (0x8832)"; case 0x8833 /* GL_DRAW_BUFFER14_ARB */:
      // return "GL_DRAW_BUFFER14_ARB (0x8833)"; case 0x8834 /*
      // GL_DRAW_BUFFER15_ARB */: return "GL_DRAW_BUFFER15_ARB (0x8834)";
      /* ----------------------- GL_ARB_draw_buffers_blend
       * ----------------------- */

      /* -------------------- GL_ARB_draw_elements_base_vertex
       * ------------------- */

      /* -------------------------- GL_ARB_draw_indirect
       * ------------------------- */

    case 0x8F3F /* GL_DRAW_INDIRECT_BUFFER */:
      return "GL_DRAW_INDIRECT_BUFFER (0x8F3F)";
    case 0x8F43 /* GL_DRAW_INDIRECT_BUFFER_BINDING */:
      return "GL_DRAW_INDIRECT_BUFFER_BINDING (0x8F43)";

      /* ------------------------- GL_ARB_draw_instanced
       * ------------------------- */

      /* -------------------- GL_ARB_explicit_attrib_location
       * -------------------- */

      /* -------------------- GL_ARB_explicit_uniform_location
       * ------------------- */

    case 0x826E /* GL_MAX_UNIFORM_LOCATIONS */:
      return "GL_MAX_UNIFORM_LOCATIONS (0x826E)";

      /* ------------------- GL_ARB_fragment_coord_conventions
       * ------------------- */

      /* --------------------- GL_ARB_fragment_layer_viewport
       * -------------------- */

      /* ------------------------ GL_ARB_fragment_program
       * ------------------------ */

    case 0x8804 /* GL_FRAGMENT_PROGRAM_ARB */:
      return "GL_FRAGMENT_PROGRAM_ARB (0x8804)";
    case 0x8807 /* GL_PROGRAM_TEX_INDIRECTIONS_ARB */:
      return "GL_PROGRAM_TEX_INDIRECTIONS_ARB (0x8807)";
    case 0x880A /* GL_PROGRAM_NATIVE_TEX_INDIRECTIONS_ARB */:
      return "GL_PROGRAM_NATIVE_TEX_INDIRECTIONS_ARB (0x880A)";
    case 0x880D /* GL_MAX_PROGRAM_TEX_INDIRECTIONS_ARB */:
      return "GL_MAX_PROGRAM_TEX_INDIRECTIONS_ARB (0x880D)";
    case 0x8810 /* GL_MAX_PROGRAM_NATIVE_TEX_INDIRECTIONS_ARB */:
      return "GL_MAX_PROGRAM_NATIVE_TEX_INDIRECTIONS_ARB (0x8810)";
      // case 0x8871 /* GL_MAX_TEXTURE_COORDS_ARB */: return
      // "GL_MAX_TEXTURE_COORDS_ARB (0x8871)"; case 0x8872 /*
      // GL_MAX_TEXTURE_IMAGE_UNITS_ARB */: return
      // "GL_MAX_TEXTURE_IMAGE_UNITS_ARB (0x8872)";

      /* --------------------- GL_ARB_fragment_program_shadow
       * -------------------- */

      /* ------------------------- GL_ARB_fragment_shader
       * ------------------------ */

      // case 0x8B30 /* GL_FRAGMENT_SHADER_ARB */: return
      // "GL_FRAGMENT_SHADER_ARB (0x8B30)"; case 0x8B49 /*
      // GL_MAX_FRAGMENT_UNIFORM_COMPONENTS_ARB */: return
      // "GL_MAX_FRAGMENT_UNIFORM_COMPONENTS_ARB (0x8B49)"; case 0x8B8B /*
      // GL_FRAGMENT_SHADER_DERIVATIVE_HINT_ARB */: return
      // "GL_FRAGMENT_SHADER_DERIVATIVE_HINT_ARB (0x8B8B)";

      /* ------------------- GL_ARB_framebuffer_no_attachments
       * ------------------- */

    case 0x9310 /* GL_FRAMEBUFFER_DEFAULT_WIDTH */:
      return "GL_FRAMEBUFFER_DEFAULT_WIDTH (0x9310)";
    case 0x9311 /* GL_FRAMEBUFFER_DEFAULT_HEIGHT */:
      return "GL_FRAMEBUFFER_DEFAULT_HEIGHT (0x9311)";
    case 0x9312 /* GL_FRAMEBUFFER_DEFAULT_LAYERS */:
      return "GL_FRAMEBUFFER_DEFAULT_LAYERS (0x9312)";
    case 0x9313 /* GL_FRAMEBUFFER_DEFAULT_SAMPLES */:
      return "GL_FRAMEBUFFER_DEFAULT_SAMPLES (0x9313)";
    case 0x9314 /* GL_FRAMEBUFFER_DEFAULT_FIXED_SAMPLE_LOCATIONS */:
      return "GL_FRAMEBUFFER_DEFAULT_FIXED_SAMPLE_LOCATIONS (0x9314)";
    case 0x9315 /* GL_MAX_FRAMEBUFFER_WIDTH */:
      return "GL_MAX_FRAMEBUFFER_WIDTH (0x9315)";
    case 0x9316 /* GL_MAX_FRAMEBUFFER_HEIGHT */:
      return "GL_MAX_FRAMEBUFFER_HEIGHT (0x9316)";
    case 0x9317 /* GL_MAX_FRAMEBUFFER_LAYERS */:
      return "GL_MAX_FRAMEBUFFER_LAYERS (0x9317)";
    case 0x9318 /* GL_MAX_FRAMEBUFFER_SAMPLES */:
      return "GL_MAX_FRAMEBUFFER_SAMPLES (0x9318)";

      /* ----------------------- GL_ARB_framebuffer_object
       * ----------------------- */

    case 0x0506 /* GL_INVALID_FRAMEBUFFER_OPERATION */:
      return "GL_INVALID_FRAMEBUFFER_OPERATION (0x0506)";
    case 0x8210 /* GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING */:
      return "GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING (0x8210)";
    case 0x8211 /* GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE */:
      return "GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE (0x8211)";
    case 0x8212 /* GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE */:
      return "GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE (0x8212)";
    case 0x8213 /* GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE */:
      return "GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE (0x8213)";
    case 0x8214 /* GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE */:
      return "GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE (0x8214)";
    case 0x8215 /* GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE */:
      return "GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE (0x8215)";
    case 0x8216 /* GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE */:
      return "GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE (0x8216)";
    case 0x8217 /* GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE */:
      return "GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE (0x8217)";
    case 0x8218 /* GL_FRAMEBUFFER_DEFAULT */:
      return "GL_FRAMEBUFFER_DEFAULT (0x8218)";
    case 0x8219 /* GL_FRAMEBUFFER_UNDEFINED */:
      return "GL_FRAMEBUFFER_UNDEFINED (0x8219)";
    case 0x821A /* GL_DEPTH_STENCIL_ATTACHMENT */:
      return "GL_DEPTH_STENCIL_ATTACHMENT (0x821A)";
    case 0x8222 /* GL_INDEX */:
      return "GL_INDEX (0x8222)";
    case 0x84E8 /* GL_MAX_RENDERBUFFER_SIZE */:
      return "GL_MAX_RENDERBUFFER_SIZE (0x84E8)";
    case 0x84F9 /* GL_DEPTH_STENCIL */:
      return "GL_DEPTH_STENCIL (0x84F9)";
    case 0x84FA /* GL_UNSIGNED_INT_24_8 */:
      return "GL_UNSIGNED_INT_24_8 (0x84FA)";
    case 0x88F0 /* GL_DEPTH24_STENCIL8 */:
      return "GL_DEPTH24_STENCIL8 (0x88F0)";
    case 0x88F1 /* GL_TEXTURE_STENCIL_SIZE */:
      return "GL_TEXTURE_STENCIL_SIZE (0x88F1)";
    case 0x8C17 /* GL_UNSIGNED_NORMALIZED */:
      return "GL_UNSIGNED_NORMALIZED (0x8C17)";
    // case 0x8C40 /* GL_SRGB */: return "GL_SRGB (0x8C40)";
    // case 0x8CA6 /* GL_DRAW_FRAMEBUFFER_BINDING */: return
    // "GL_DRAW_FRAMEBUFFER_BINDING (0x8CA6)";
    case 0x8CA6 /* GL_FRAMEBUFFER_BINDING */:
      return "GL_FRAMEBUFFER_BINDING (0x8CA6)";
    case 0x8CA7 /* GL_RENDERBUFFER_BINDING */:
      return "GL_RENDERBUFFER_BINDING (0x8CA7)";
    case 0x8CA8 /* GL_READ_FRAMEBUFFER */:
      return "GL_READ_FRAMEBUFFER (0x8CA8)";
    case 0x8CA9 /* GL_DRAW_FRAMEBUFFER */:
      return "GL_DRAW_FRAMEBUFFER (0x8CA9)";
    case 0x8CAA /* GL_READ_FRAMEBUFFER_BINDING */:
      return "GL_READ_FRAMEBUFFER_BINDING (0x8CAA)";
    case 0x8CAB /* GL_RENDERBUFFER_SAMPLES */:
      return "GL_RENDERBUFFER_SAMPLES (0x8CAB)";
    case 0x8CD0 /* GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE */:
      return "GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE (0x8CD0)";
    case 0x8CD1 /* GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME */:
      return "GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME (0x8CD1)";
    case 0x8CD2 /* GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL */:
      return "GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL (0x8CD2)";
    case 0x8CD3 /* GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE */:
      return "GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE (0x8CD3)";
    case 0x8CD4 /* GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER */:
      return "GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER (0x8CD4)";
    case 0x8CD5 /* GL_FRAMEBUFFER_COMPLETE */:
      return "GL_FRAMEBUFFER_COMPLETE (0x8CD5)";
    case 0x8CD6 /* GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT */:
      return "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT (0x8CD6)";
    case 0x8CD7 /* GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT */:
      return "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT (0x8CD7)";
    case 0x8CDB /* GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER */:
      return "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER (0x8CDB)";
    case 0x8CDC /* GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER */:
      return "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER (0x8CDC)";
    case 0x8CDD /* GL_FRAMEBUFFER_UNSUPPORTED */:
      return "GL_FRAMEBUFFER_UNSUPPORTED (0x8CDD)";
    case 0x8CDF /* GL_MAX_COLOR_ATTACHMENTS */:
      return "GL_MAX_COLOR_ATTACHMENTS (0x8CDF)";
    case 0x8CE0 /* GL_COLOR_ATTACHMENT0 */:
      return "GL_COLOR_ATTACHMENT0 (0x8CE0)";
    case 0x8CE1 /* GL_COLOR_ATTACHMENT1 */:
      return "GL_COLOR_ATTACHMENT1 (0x8CE1)";
    case 0x8CE2 /* GL_COLOR_ATTACHMENT2 */:
      return "GL_COLOR_ATTACHMENT2 (0x8CE2)";
    case 0x8CE3 /* GL_COLOR_ATTACHMENT3 */:
      return "GL_COLOR_ATTACHMENT3 (0x8CE3)";
    case 0x8CE4 /* GL_COLOR_ATTACHMENT4 */:
      return "GL_COLOR_ATTACHMENT4 (0x8CE4)";
    case 0x8CE5 /* GL_COLOR_ATTACHMENT5 */:
      return "GL_COLOR_ATTACHMENT5 (0x8CE5)";
    case 0x8CE6 /* GL_COLOR_ATTACHMENT6 */:
      return "GL_COLOR_ATTACHMENT6 (0x8CE6)";
    case 0x8CE7 /* GL_COLOR_ATTACHMENT7 */:
      return "GL_COLOR_ATTACHMENT7 (0x8CE7)";
    case 0x8CE8 /* GL_COLOR_ATTACHMENT8 */:
      return "GL_COLOR_ATTACHMENT8 (0x8CE8)";
    case 0x8CE9 /* GL_COLOR_ATTACHMENT9 */:
      return "GL_COLOR_ATTACHMENT9 (0x8CE9)";
    case 0x8CEA /* GL_COLOR_ATTACHMENT10 */:
      return "GL_COLOR_ATTACHMENT10 (0x8CEA)";
    case 0x8CEB /* GL_COLOR_ATTACHMENT11 */:
      return "GL_COLOR_ATTACHMENT11 (0x8CEB)";
    case 0x8CEC /* GL_COLOR_ATTACHMENT12 */:
      return "GL_COLOR_ATTACHMENT12 (0x8CEC)";
    case 0x8CED /* GL_COLOR_ATTACHMENT13 */:
      return "GL_COLOR_ATTACHMENT13 (0x8CED)";
    case 0x8CEE /* GL_COLOR_ATTACHMENT14 */:
      return "GL_COLOR_ATTACHMENT14 (0x8CEE)";
    case 0x8CEF /* GL_COLOR_ATTACHMENT15 */:
      return "GL_COLOR_ATTACHMENT15 (0x8CEF)";
    case 0x8D00 /* GL_DEPTH_ATTACHMENT */:
      return "GL_DEPTH_ATTACHMENT (0x8D00)";
    case 0x8D20 /* GL_STENCIL_ATTACHMENT */:
      return "GL_STENCIL_ATTACHMENT (0x8D20)";
    case 0x8D40 /* GL_FRAMEBUFFER */:
      return "GL_FRAMEBUFFER (0x8D40)";
    case 0x8D41 /* GL_RENDERBUFFER */:
      return "GL_RENDERBUFFER (0x8D41)";
    case 0x8D42 /* GL_RENDERBUFFER_WIDTH */:
      return "GL_RENDERBUFFER_WIDTH (0x8D42)";
    case 0x8D43 /* GL_RENDERBUFFER_HEIGHT */:
      return "GL_RENDERBUFFER_HEIGHT (0x8D43)";
    case 0x8D44 /* GL_RENDERBUFFER_INTERNAL_FORMAT */:
      return "GL_RENDERBUFFER_INTERNAL_FORMAT (0x8D44)";
    case 0x8D46 /* GL_STENCIL_INDEX1 */:
      return "GL_STENCIL_INDEX1 (0x8D46)";
    case 0x8D47 /* GL_STENCIL_INDEX4 */:
      return "GL_STENCIL_INDEX4 (0x8D47)";
    case 0x8D48 /* GL_STENCIL_INDEX8 */:
      return "GL_STENCIL_INDEX8 (0x8D48)";
    case 0x8D49 /* GL_STENCIL_INDEX16 */:
      return "GL_STENCIL_INDEX16 (0x8D49)";
    case 0x8D50 /* GL_RENDERBUFFER_RED_SIZE */:
      return "GL_RENDERBUFFER_RED_SIZE (0x8D50)";
    case 0x8D51 /* GL_RENDERBUFFER_GREEN_SIZE */:
      return "GL_RENDERBUFFER_GREEN_SIZE (0x8D51)";
    case 0x8D52 /* GL_RENDERBUFFER_BLUE_SIZE */:
      return "GL_RENDERBUFFER_BLUE_SIZE (0x8D52)";
    case 0x8D53 /* GL_RENDERBUFFER_ALPHA_SIZE */:
      return "GL_RENDERBUFFER_ALPHA_SIZE (0x8D53)";
    case 0x8D54 /* GL_RENDERBUFFER_DEPTH_SIZE */:
      return "GL_RENDERBUFFER_DEPTH_SIZE (0x8D54)";
    case 0x8D55 /* GL_RENDERBUFFER_STENCIL_SIZE */:
      return "GL_RENDERBUFFER_STENCIL_SIZE (0x8D55)";
    case 0x8D56 /* GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE */:
      return "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE (0x8D56)";
    case 0x8D57 /* GL_MAX_SAMPLES */:
      return "GL_MAX_SAMPLES (0x8D57)";

      /* ------------------------ GL_ARB_framebuffer_sRGB
       * ------------------------ */

    case 0x8DB9 /* GL_FRAMEBUFFER_SRGB */:
      return "GL_FRAMEBUFFER_SRGB (0x8DB9)";

    /* ------------------------ GL_ARB_geometry_shader4 ------------------------
     */

    // case 0xA /* GL_LINES_ADJACENCY_ARB */: return "GL_LINES_ADJACENCY_ARB
    // (0xA)"; case 0xB /* GL_LINE_STRIP_ADJACENCY_ARB */: return
    // "GL_LINE_STRIP_ADJACENCY_ARB (0xB)"; case 0xC /*
    // GL_TRIANGLES_ADJACENCY_ARB */: return "GL_TRIANGLES_ADJACENCY_ARB (0xC)";
    // case 0xD /* GL_TRIANGLE_STRIP_ADJACENCY_ARB */: return
    // "GL_TRIANGLE_STRIP_ADJACENCY_ARB (0xD)"; case 0x8C29 /*
    // GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS_ARB */: return
    // "GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS_ARB (0x8C29)"; case 0x8CD4 /*
    // GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER */: return
    // "GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER (0x8CD4)"; case 0x8DA7 /*
    // GL_FRAMEBUFFER_ATTACHMENT_LAYERED_ARB */: return
    // "GL_FRAMEBUFFER_ATTACHMENT_LAYERED_ARB (0x8DA7)"; case 0x8DA8 /*
    // GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS_ARB */: return
    // "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS_ARB (0x8DA8)";
    case 0x8DA9 /* GL_FRAMEBUFFER_INCOMPLETE_LAYER_COUNT_ARB */:
      return "GL_FRAMEBUFFER_INCOMPLETE_LAYER_COUNT_ARB (0x8DA9)";
    // case 0x8DD9 /* GL_GEOMETRY_SHADER_ARB */: return "GL_GEOMETRY_SHADER_ARB
    // (0x8DD9)";
    case 0x8DDA /* GL_GEOMETRY_VERTICES_OUT_ARB */:
      return "GL_GEOMETRY_VERTICES_OUT_ARB (0x8DDA)";
    case 0x8DDB /* GL_GEOMETRY_INPUT_TYPE_ARB */:
      return "GL_GEOMETRY_INPUT_TYPE_ARB (0x8DDB)";
    case 0x8DDC /* GL_GEOMETRY_OUTPUT_TYPE_ARB */:
      return "GL_GEOMETRY_OUTPUT_TYPE_ARB (0x8DDC)";
    case 0x8DDD /* GL_MAX_GEOMETRY_VARYING_COMPONENTS_ARB */:
      return "GL_MAX_GEOMETRY_VARYING_COMPONENTS_ARB (0x8DDD)";
    case 0x8DDE /* GL_MAX_VERTEX_VARYING_COMPONENTS_ARB */:
      return "GL_MAX_VERTEX_VARYING_COMPONENTS_ARB (0x8DDE)";
      // case 0x8DDF /* GL_MAX_GEOMETRY_UNIFORM_COMPONENTS_ARB */: return
      // "GL_MAX_GEOMETRY_UNIFORM_COMPONENTS_ARB (0x8DDF)"; case 0x8DE0 /*
      // GL_MAX_GEOMETRY_OUTPUT_VERTICES_ARB */: return
      // "GL_MAX_GEOMETRY_OUTPUT_VERTICES_ARB (0x8DE0)"; case 0x8DE1 /*
      // GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS_ARB */: return
      // "GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS_ARB (0x8DE1)";

      /* ----------------------- GL_ARB_get_program_binary
       * ----------------------- */

    case 0x8257 /* GL_PROGRAM_BINARY_RETRIEVABLE_HINT */:
      return "GL_PROGRAM_BINARY_RETRIEVABLE_HINT (0x8257)";
    case 0x8741 /* GL_PROGRAM_BINARY_LENGTH */:
      return "GL_PROGRAM_BINARY_LENGTH (0x8741)";
    case 0x87FE /* GL_NUM_PROGRAM_BINARY_FORMATS */:
      return "GL_NUM_PROGRAM_BINARY_FORMATS (0x87FE)";
    case 0x87FF /* GL_PROGRAM_BINARY_FORMATS */:
      return "GL_PROGRAM_BINARY_FORMATS (0x87FF)";

    /* --------------------------- GL_ARB_gpu_shader5 --------------------------
     */

    // case 0x887F /* GL_GEOMETRY_SHADER_INVOCATIONS */: return
    // "GL_GEOMETRY_SHADER_INVOCATIONS (0x887F)"; case 0x8E5A /*
    // GL_MAX_GEOMETRY_SHADER_INVOCATIONS */: return
    // "GL_MAX_GEOMETRY_SHADER_INVOCATIONS (0x8E5A)"; case 0x8E5B /*
    // GL_MIN_FRAGMENT_INTERPOLATION_OFFSET */: return
    // "GL_MIN_FRAGMENT_INTERPOLATION_OFFSET (0x8E5B)"; case 0x8E5C /*
    // GL_MAX_FRAGMENT_INTERPOLATION_OFFSET */: return
    // "GL_MAX_FRAGMENT_INTERPOLATION_OFFSET (0x8E5C)"; case 0x8E5D /*
    // GL_FRAGMENT_INTERPOLATION_OFFSET_BITS */: return
    // "GL_FRAGMENT_INTERPOLATION_OFFSET_BITS (0x8E5D)";
    case 0x8E71 /* GL_MAX_VERTEX_STREAMS */:
      return "GL_MAX_VERTEX_STREAMS (0x8E71)";

      /* ------------------------- GL_ARB_gpu_shader_fp64
       * ------------------------ */

    case 0x8F46 /* GL_DOUBLE_MAT2 */:
      return "GL_DOUBLE_MAT2 (0x8F46)";
    case 0x8F47 /* GL_DOUBLE_MAT3 */:
      return "GL_DOUBLE_MAT3 (0x8F47)";
    case 0x8F48 /* GL_DOUBLE_MAT4 */:
      return "GL_DOUBLE_MAT4 (0x8F48)";
    case 0x8F49 /* GL_DOUBLE_MAT2x3 */:
      return "GL_DOUBLE_MAT2x3 (0x8F49)";
    case 0x8F4A /* GL_DOUBLE_MAT2x4 */:
      return "GL_DOUBLE_MAT2x4 (0x8F4A)";
    case 0x8F4B /* GL_DOUBLE_MAT3x2 */:
      return "GL_DOUBLE_MAT3x2 (0x8F4B)";
    case 0x8F4C /* GL_DOUBLE_MAT3x4 */:
      return "GL_DOUBLE_MAT3x4 (0x8F4C)";
    case 0x8F4D /* GL_DOUBLE_MAT4x2 */:
      return "GL_DOUBLE_MAT4x2 (0x8F4D)";
    case 0x8F4E /* GL_DOUBLE_MAT4x3 */:
      return "GL_DOUBLE_MAT4x3 (0x8F4E)";
    case 0x8FFC /* GL_DOUBLE_VEC2 */:
      return "GL_DOUBLE_VEC2 (0x8FFC)";
    case 0x8FFD /* GL_DOUBLE_VEC3 */:
      return "GL_DOUBLE_VEC3 (0x8FFD)";
    case 0x8FFE /* GL_DOUBLE_VEC4 */:
      return "GL_DOUBLE_VEC4 (0x8FFE)";

      /* ------------------------ GL_ARB_half_float_pixel
       * ------------------------ */

      // case 0x140B /* GL_HALF_FLOAT_ARB */: return "GL_HALF_FLOAT_ARB
      // (0x140B)";

      /* ------------------------ GL_ARB_half_float_vertex
       * ----------------------- */

    case 0x140B /* GL_HALF_FLOAT */:
      return "GL_HALF_FLOAT (0x140B)";

      /* ----------------------------- GL_ARB_imaging
       * ---------------------------- */

    case 0x8001 /* GL_CONSTANT_COLOR */:
      return "GL_CONSTANT_COLOR (0x8001)";
    case 0x8002 /* GL_ONE_MINUS_CONSTANT_COLOR */:
      return "GL_ONE_MINUS_CONSTANT_COLOR (0x8002)";
    case 0x8003 /* GL_CONSTANT_ALPHA */:
      return "GL_CONSTANT_ALPHA (0x8003)";
    case 0x8004 /* GL_ONE_MINUS_CONSTANT_ALPHA */:
      return "GL_ONE_MINUS_CONSTANT_ALPHA (0x8004)";
    case 0x8005 /* GL_BLEND_COLOR */:
      return "GL_BLEND_COLOR (0x8005)";
    case 0x8006 /* GL_FUNC_ADD */:
      return "GL_FUNC_ADD (0x8006)";
    case 0x8007 /* GL_MIN */:
      return "GL_MIN (0x8007)";
    case 0x8008 /* GL_MAX */:
      return "GL_MAX (0x8008)";
    case 0x8009 /* GL_BLEND_EQUATION */:
      return "GL_BLEND_EQUATION (0x8009)";
    case 0x800A /* GL_FUNC_SUBTRACT */:
      return "GL_FUNC_SUBTRACT (0x800A)";
    case 0x800B /* GL_FUNC_REVERSE_SUBTRACT */:
      return "GL_FUNC_REVERSE_SUBTRACT (0x800B)";
    case 0x8010 /* GL_CONVOLUTION_1D */:
      return "GL_CONVOLUTION_1D (0x8010)";
    case 0x8011 /* GL_CONVOLUTION_2D */:
      return "GL_CONVOLUTION_2D (0x8011)";
    case 0x8012 /* GL_SEPARABLE_2D */:
      return "GL_SEPARABLE_2D (0x8012)";
    case 0x8013 /* GL_CONVOLUTION_BORDER_MODE */:
      return "GL_CONVOLUTION_BORDER_MODE (0x8013)";
    case 0x8014 /* GL_CONVOLUTION_FILTER_SCALE */:
      return "GL_CONVOLUTION_FILTER_SCALE (0x8014)";
    case 0x8015 /* GL_CONVOLUTION_FILTER_BIAS */:
      return "GL_CONVOLUTION_FILTER_BIAS (0x8015)";
    case 0x8016 /* GL_REDUCE */:
      return "GL_REDUCE (0x8016)";
    case 0x8017 /* GL_CONVOLUTION_FORMAT */:
      return "GL_CONVOLUTION_FORMAT (0x8017)";
    case 0x8018 /* GL_CONVOLUTION_WIDTH */:
      return "GL_CONVOLUTION_WIDTH (0x8018)";
    case 0x8019 /* GL_CONVOLUTION_HEIGHT */:
      return "GL_CONVOLUTION_HEIGHT (0x8019)";
    case 0x801A /* GL_MAX_CONVOLUTION_WIDTH */:
      return "GL_MAX_CONVOLUTION_WIDTH (0x801A)";
    case 0x801B /* GL_MAX_CONVOLUTION_HEIGHT */:
      return "GL_MAX_CONVOLUTION_HEIGHT (0x801B)";
    case 0x801C /* GL_POST_CONVOLUTION_RED_SCALE */:
      return "GL_POST_CONVOLUTION_RED_SCALE (0x801C)";
    case 0x801D /* GL_POST_CONVOLUTION_GREEN_SCALE */:
      return "GL_POST_CONVOLUTION_GREEN_SCALE (0x801D)";
    case 0x801E /* GL_POST_CONVOLUTION_BLUE_SCALE */:
      return "GL_POST_CONVOLUTION_BLUE_SCALE (0x801E)";
    case 0x801F /* GL_POST_CONVOLUTION_ALPHA_SCALE */:
      return "GL_POST_CONVOLUTION_ALPHA_SCALE (0x801F)";
    case 0x8020 /* GL_POST_CONVOLUTION_RED_BIAS */:
      return "GL_POST_CONVOLUTION_RED_BIAS (0x8020)";
    case 0x8021 /* GL_POST_CONVOLUTION_GREEN_BIAS */:
      return "GL_POST_CONVOLUTION_GREEN_BIAS (0x8021)";
    case 0x8022 /* GL_POST_CONVOLUTION_BLUE_BIAS */:
      return "GL_POST_CONVOLUTION_BLUE_BIAS (0x8022)";
    case 0x8023 /* GL_POST_CONVOLUTION_ALPHA_BIAS */:
      return "GL_POST_CONVOLUTION_ALPHA_BIAS (0x8023)";
    case 0x8024 /* GL_HISTOGRAM */:
      return "GL_HISTOGRAM (0x8024)";
    case 0x8025 /* GL_PROXY_HISTOGRAM */:
      return "GL_PROXY_HISTOGRAM (0x8025)";
    case 0x8026 /* GL_HISTOGRAM_WIDTH */:
      return "GL_HISTOGRAM_WIDTH (0x8026)";
    case 0x8027 /* GL_HISTOGRAM_FORMAT */:
      return "GL_HISTOGRAM_FORMAT (0x8027)";
    case 0x8028 /* GL_HISTOGRAM_RED_SIZE */:
      return "GL_HISTOGRAM_RED_SIZE (0x8028)";
    case 0x8029 /* GL_HISTOGRAM_GREEN_SIZE */:
      return "GL_HISTOGRAM_GREEN_SIZE (0x8029)";
    case 0x802A /* GL_HISTOGRAM_BLUE_SIZE */:
      return "GL_HISTOGRAM_BLUE_SIZE (0x802A)";
    case 0x802B /* GL_HISTOGRAM_ALPHA_SIZE */:
      return "GL_HISTOGRAM_ALPHA_SIZE (0x802B)";
    case 0x802C /* GL_HISTOGRAM_LUMINANCE_SIZE */:
      return "GL_HISTOGRAM_LUMINANCE_SIZE (0x802C)";
    case 0x802D /* GL_HISTOGRAM_SINK */:
      return "GL_HISTOGRAM_SINK (0x802D)";
    case 0x802E /* GL_MINMAX */:
      return "GL_MINMAX (0x802E)";
    case 0x802F /* GL_MINMAX_FORMAT */:
      return "GL_MINMAX_FORMAT (0x802F)";
    case 0x8030 /* GL_MINMAX_SINK */:
      return "GL_MINMAX_SINK (0x8030)";
    case 0x8031 /* GL_TABLE_TOO_LARGE */:
      return "GL_TABLE_TOO_LARGE (0x8031)";
    case 0x80B1 /* GL_COLOR_MATRIX */:
      return "GL_COLOR_MATRIX (0x80B1)";
    case 0x80B2 /* GL_COLOR_MATRIX_STACK_DEPTH */:
      return "GL_COLOR_MATRIX_STACK_DEPTH (0x80B2)";
    case 0x80B3 /* GL_MAX_COLOR_MATRIX_STACK_DEPTH */:
      return "GL_MAX_COLOR_MATRIX_STACK_DEPTH (0x80B3)";
    case 0x80B4 /* GL_POST_COLOR_MATRIX_RED_SCALE */:
      return "GL_POST_COLOR_MATRIX_RED_SCALE (0x80B4)";
    case 0x80B5 /* GL_POST_COLOR_MATRIX_GREEN_SCALE */:
      return "GL_POST_COLOR_MATRIX_GREEN_SCALE (0x80B5)";
    case 0x80B6 /* GL_POST_COLOR_MATRIX_BLUE_SCALE */:
      return "GL_POST_COLOR_MATRIX_BLUE_SCALE (0x80B6)";
    case 0x80B7 /* GL_POST_COLOR_MATRIX_ALPHA_SCALE */:
      return "GL_POST_COLOR_MATRIX_ALPHA_SCALE (0x80B7)";
    case 0x80B8 /* GL_POST_COLOR_MATRIX_RED_BIAS */:
      return "GL_POST_COLOR_MATRIX_RED_BIAS (0x80B8)";
    case 0x80B9 /* GL_POST_COLOR_MATRIX_GREEN_BIAS */:
      return "GL_POST_COLOR_MATRIX_GREEN_BIAS (0x80B9)";
    case 0x80BA /* GL_POST_COLOR_MATRIX_BLUE_BIAS */:
      return "GL_POST_COLOR_MATRIX_BLUE_BIAS (0x80BA)";
    case 0x80BB /* GL_POST_COLOR_MATRIX_ALPHA_BIAS */:
      return "GL_POST_COLOR_MATRIX_ALPHA_BIAS (0x80BB)";
    case 0x80D0 /* GL_COLOR_TABLE */:
      return "GL_COLOR_TABLE (0x80D0)";
    case 0x80D1 /* GL_POST_CONVOLUTION_COLOR_TABLE */:
      return "GL_POST_CONVOLUTION_COLOR_TABLE (0x80D1)";
    case 0x80D2 /* GL_POST_COLOR_MATRIX_COLOR_TABLE */:
      return "GL_POST_COLOR_MATRIX_COLOR_TABLE (0x80D2)";
    case 0x80D3 /* GL_PROXY_COLOR_TABLE */:
      return "GL_PROXY_COLOR_TABLE (0x80D3)";
    case 0x80D4 /* GL_PROXY_POST_CONVOLUTION_COLOR_TABLE */:
      return "GL_PROXY_POST_CONVOLUTION_COLOR_TABLE (0x80D4)";
    case 0x80D5 /* GL_PROXY_POST_COLOR_MATRIX_COLOR_TABLE */:
      return "GL_PROXY_POST_COLOR_MATRIX_COLOR_TABLE (0x80D5)";
    case 0x80D6 /* GL_COLOR_TABLE_SCALE */:
      return "GL_COLOR_TABLE_SCALE (0x80D6)";
    case 0x80D7 /* GL_COLOR_TABLE_BIAS */:
      return "GL_COLOR_TABLE_BIAS (0x80D7)";
    case 0x80D8 /* GL_COLOR_TABLE_FORMAT */:
      return "GL_COLOR_TABLE_FORMAT (0x80D8)";
    case 0x80D9 /* GL_COLOR_TABLE_WIDTH */:
      return "GL_COLOR_TABLE_WIDTH (0x80D9)";
    case 0x80DA /* GL_COLOR_TABLE_RED_SIZE */:
      return "GL_COLOR_TABLE_RED_SIZE (0x80DA)";
    case 0x80DB /* GL_COLOR_TABLE_GREEN_SIZE */:
      return "GL_COLOR_TABLE_GREEN_SIZE (0x80DB)";
    case 0x80DC /* GL_COLOR_TABLE_BLUE_SIZE */:
      return "GL_COLOR_TABLE_BLUE_SIZE (0x80DC)";
    case 0x80DD /* GL_COLOR_TABLE_ALPHA_SIZE */:
      return "GL_COLOR_TABLE_ALPHA_SIZE (0x80DD)";
    case 0x80DE /* GL_COLOR_TABLE_LUMINANCE_SIZE */:
      return "GL_COLOR_TABLE_LUMINANCE_SIZE (0x80DE)";
    case 0x80DF /* GL_COLOR_TABLE_INTENSITY_SIZE */:
      return "GL_COLOR_TABLE_INTENSITY_SIZE (0x80DF)";
    case 0x8150 /* GL_IGNORE_BORDER */:
      return "GL_IGNORE_BORDER (0x8150)";
    case 0x8151 /* GL_CONSTANT_BORDER */:
      return "GL_CONSTANT_BORDER (0x8151)";
    case 0x8152 /* GL_WRAP_BORDER */:
      return "GL_WRAP_BORDER (0x8152)";
    case 0x8153 /* GL_REPLICATE_BORDER */:
      return "GL_REPLICATE_BORDER (0x8153)";
    case 0x8154 /* GL_CONVOLUTION_BORDER_COLOR */:
      return "GL_CONVOLUTION_BORDER_COLOR (0x8154)";

      /* ------------------------ GL_ARB_instanced_arrays
       * ------------------------ */

      // case 0x88FE /* GL_VERTEX_ATTRIB_ARRAY_DIVISOR_ARB */: return
      // "GL_VERTEX_ATTRIB_ARRAY_DIVISOR_ARB (0x88FE)";

      /* ---------------------- GL_ARB_internalformat_query
       * ---------------------- */

    case 0x9380 /* GL_NUM_SAMPLE_COUNTS */:
      return "GL_NUM_SAMPLE_COUNTS (0x9380)"; /* ----------------------
                                                 GL_ARB_internalformat_query2
                                                 --------------------- */

    // case 0x0DE0 /* GL_TEXTURE_1D */: return "GL_TEXTURE_1D (0x0DE0)";
    // case 0x0DE1 /* GL_TEXTURE_2D */: return "GL_TEXTURE_2D (0x0DE1)";
    // case 0x806F /* GL_TEXTURE_3D */: return "GL_TEXTURE_3D (0x806F)";
    // case 0x80A9 /* GL_SAMPLES */: return "GL_SAMPLES (0x80A9)";
    case 0x826F /* GL_INTERNALFORMAT_SUPPORTED */:
      return "GL_INTERNALFORMAT_SUPPORTED (0x826F)";
    case 0x8270 /* GL_INTERNALFORMAT_PREFERRED */:
      return "GL_INTERNALFORMAT_PREFERRED (0x8270)";
    case 0x8271 /* GL_INTERNALFORMAT_RED_SIZE */:
      return "GL_INTERNALFORMAT_RED_SIZE (0x8271)";
    case 0x8272 /* GL_INTERNALFORMAT_GREEN_SIZE */:
      return "GL_INTERNALFORMAT_GREEN_SIZE (0x8272)";
    case 0x8273 /* GL_INTERNALFORMAT_BLUE_SIZE */:
      return "GL_INTERNALFORMAT_BLUE_SIZE (0x8273)";
    case 0x8274 /* GL_INTERNALFORMAT_ALPHA_SIZE */:
      return "GL_INTERNALFORMAT_ALPHA_SIZE (0x8274)";
    case 0x8275 /* GL_INTERNALFORMAT_DEPTH_SIZE */:
      return "GL_INTERNALFORMAT_DEPTH_SIZE (0x8275)";
    case 0x8276 /* GL_INTERNALFORMAT_STENCIL_SIZE */:
      return "GL_INTERNALFORMAT_STENCIL_SIZE (0x8276)";
    case 0x8277 /* GL_INTERNALFORMAT_SHARED_SIZE */:
      return "GL_INTERNALFORMAT_SHARED_SIZE (0x8277)";
    case 0x8278 /* GL_INTERNALFORMAT_RED_TYPE */:
      return "GL_INTERNALFORMAT_RED_TYPE (0x8278)";
    case 0x8279 /* GL_INTERNALFORMAT_GREEN_TYPE */:
      return "GL_INTERNALFORMAT_GREEN_TYPE (0x8279)";
    case 0x827A /* GL_INTERNALFORMAT_BLUE_TYPE */:
      return "GL_INTERNALFORMAT_BLUE_TYPE (0x827A)";
    case 0x827B /* GL_INTERNALFORMAT_ALPHA_TYPE */:
      return "GL_INTERNALFORMAT_ALPHA_TYPE (0x827B)";
    case 0x827C /* GL_INTERNALFORMAT_DEPTH_TYPE */:
      return "GL_INTERNALFORMAT_DEPTH_TYPE (0x827C)";
    case 0x827D /* GL_INTERNALFORMAT_STENCIL_TYPE */:
      return "GL_INTERNALFORMAT_STENCIL_TYPE (0x827D)";
    case 0x827E /* GL_MAX_WIDTH */:
      return "GL_MAX_WIDTH (0x827E)";
    case 0x827F /* GL_MAX_HEIGHT */:
      return "GL_MAX_HEIGHT (0x827F)";
    case 0x8280 /* GL_MAX_DEPTH */:
      return "GL_MAX_DEPTH (0x8280)";
    case 0x8281 /* GL_MAX_LAYERS */:
      return "GL_MAX_LAYERS (0x8281)";
    case 0x8282 /* GL_MAX_COMBINED_DIMENSIONS */:
      return "GL_MAX_COMBINED_DIMENSIONS (0x8282)";
    case 0x8283 /* GL_COLOR_COMPONENTS */:
      return "GL_COLOR_COMPONENTS (0x8283)";
    case 0x8284 /* GL_DEPTH_COMPONENTS */:
      return "GL_DEPTH_COMPONENTS (0x8284)";
    case 0x8285 /* GL_STENCIL_COMPONENTS */:
      return "GL_STENCIL_COMPONENTS (0x8285)";
    case 0x8286 /* GL_COLOR_RENDERABLE */:
      return "GL_COLOR_RENDERABLE (0x8286)";
    case 0x8287 /* GL_DEPTH_RENDERABLE */:
      return "GL_DEPTH_RENDERABLE (0x8287)";
    case 0x8288 /* GL_STENCIL_RENDERABLE */:
      return "GL_STENCIL_RENDERABLE (0x8288)";
    case 0x8289 /* GL_FRAMEBUFFER_RENDERABLE */:
      return "GL_FRAMEBUFFER_RENDERABLE (0x8289)";
    case 0x828A /* GL_FRAMEBUFFER_RENDERABLE_LAYERED */:
      return "GL_FRAMEBUFFER_RENDERABLE_LAYERED (0x828A)";
    case 0x828B /* GL_FRAMEBUFFER_BLEND */:
      return "GL_FRAMEBUFFER_BLEND (0x828B)";
    case 0x828C /* GL_READ_PIXELS */:
      return "GL_READ_PIXELS (0x828C)";
    case 0x828D /* GL_READ_PIXELS_FORMAT */:
      return "GL_READ_PIXELS_FORMAT (0x828D)";
    case 0x828E /* GL_READ_PIXELS_TYPE */:
      return "GL_READ_PIXELS_TYPE (0x828E)";
    case 0x828F /* GL_TEXTURE_IMAGE_FORMAT */:
      return "GL_TEXTURE_IMAGE_FORMAT (0x828F)";
    case 0x8290 /* GL_TEXTURE_IMAGE_TYPE */:
      return "GL_TEXTURE_IMAGE_TYPE (0x8290)";
    case 0x8291 /* GL_GET_TEXTURE_IMAGE_FORMAT */:
      return "GL_GET_TEXTURE_IMAGE_FORMAT (0x8291)";
    case 0x8292 /* GL_GET_TEXTURE_IMAGE_TYPE */:
      return "GL_GET_TEXTURE_IMAGE_TYPE (0x8292)";
    case 0x8293 /* GL_MIPMAP */:
      return "GL_MIPMAP (0x8293)";
    case 0x8294 /* GL_MANUAL_GENERATE_MIPMAP */:
      return "GL_MANUAL_GENERATE_MIPMAP (0x8294)";
    case 0x8295 /* GL_AUTO_GENERATE_MIPMAP */:
      return "GL_AUTO_GENERATE_MIPMAP (0x8295)";
    case 0x8296 /* GL_COLOR_ENCODING */:
      return "GL_COLOR_ENCODING (0x8296)";
    case 0x8297 /* GL_SRGB_READ */:
      return "GL_SRGB_READ (0x8297)";
    case 0x8298 /* GL_SRGB_WRITE */:
      return "GL_SRGB_WRITE (0x8298)";
    case 0x8299 /* GL_SRGB_DECODE_ARB */:
      return "GL_SRGB_DECODE_ARB (0x8299)";
    case 0x829A /* GL_FILTER */:
      return "GL_FILTER (0x829A)";
    case 0x829B /* GL_VERTEX_TEXTURE */:
      return "GL_VERTEX_TEXTURE (0x829B)";
    case 0x829C /* GL_TESS_CONTROL_TEXTURE */:
      return "GL_TESS_CONTROL_TEXTURE (0x829C)";
    case 0x829D /* GL_TESS_EVALUATION_TEXTURE */:
      return "GL_TESS_EVALUATION_TEXTURE (0x829D)";
    case 0x829E /* GL_GEOMETRY_TEXTURE */:
      return "GL_GEOMETRY_TEXTURE (0x829E)";
    case 0x829F /* GL_FRAGMENT_TEXTURE */:
      return "GL_FRAGMENT_TEXTURE (0x829F)";
    case 0x82A0 /* GL_COMPUTE_TEXTURE */:
      return "GL_COMPUTE_TEXTURE (0x82A0)";
    case 0x82A1 /* GL_TEXTURE_SHADOW */:
      return "GL_TEXTURE_SHADOW (0x82A1)";
    case 0x82A2 /* GL_TEXTURE_GATHER */:
      return "GL_TEXTURE_GATHER (0x82A2)";
    case 0x82A3 /* GL_TEXTURE_GATHER_SHADOW */:
      return "GL_TEXTURE_GATHER_SHADOW (0x82A3)";
    case 0x82A4 /* GL_SHADER_IMAGE_LOAD */:
      return "GL_SHADER_IMAGE_LOAD (0x82A4)";
    case 0x82A5 /* GL_SHADER_IMAGE_STORE */:
      return "GL_SHADER_IMAGE_STORE (0x82A5)";
    case 0x82A6 /* GL_SHADER_IMAGE_ATOMIC */:
      return "GL_SHADER_IMAGE_ATOMIC (0x82A6)";
    case 0x82A7 /* GL_IMAGE_TEXEL_SIZE */:
      return "GL_IMAGE_TEXEL_SIZE (0x82A7)";
    case 0x82A8 /* GL_IMAGE_COMPATIBILITY_CLASS */:
      return "GL_IMAGE_COMPATIBILITY_CLASS (0x82A8)";
    case 0x82A9 /* GL_IMAGE_PIXEL_FORMAT */:
      return "GL_IMAGE_PIXEL_FORMAT (0x82A9)";
    case 0x82AA /* GL_IMAGE_PIXEL_TYPE */:
      return "GL_IMAGE_PIXEL_TYPE (0x82AA)";
    case 0x82AC /* GL_SIMULTANEOUS_TEXTURE_AND_DEPTH_TEST */:
      return "GL_SIMULTANEOUS_TEXTURE_AND_DEPTH_TEST (0x82AC)";
    case 0x82AD /* GL_SIMULTANEOUS_TEXTURE_AND_STENCIL_TEST */:
      return "GL_SIMULTANEOUS_TEXTURE_AND_STENCIL_TEST (0x82AD)";
    case 0x82AE /* GL_SIMULTANEOUS_TEXTURE_AND_DEPTH_WRITE */:
      return "GL_SIMULTANEOUS_TEXTURE_AND_DEPTH_WRITE (0x82AE)";
    case 0x82AF /* GL_SIMULTANEOUS_TEXTURE_AND_STENCIL_WRITE */:
      return "GL_SIMULTANEOUS_TEXTURE_AND_STENCIL_WRITE (0x82AF)";
    case 0x82B1 /* GL_TEXTURE_COMPRESSED_BLOCK_WIDTH */:
      return "GL_TEXTURE_COMPRESSED_BLOCK_WIDTH (0x82B1)";
    case 0x82B2 /* GL_TEXTURE_COMPRESSED_BLOCK_HEIGHT */:
      return "GL_TEXTURE_COMPRESSED_BLOCK_HEIGHT (0x82B2)";
    case 0x82B3 /* GL_TEXTURE_COMPRESSED_BLOCK_SIZE */:
      return "GL_TEXTURE_COMPRESSED_BLOCK_SIZE (0x82B3)";
    case 0x82B4 /* GL_CLEAR_BUFFER */:
      return "GL_CLEAR_BUFFER (0x82B4)";
    case 0x82B5 /* GL_TEXTURE_VIEW */:
      return "GL_TEXTURE_VIEW (0x82B5)";
    case 0x82B6 /* GL_VIEW_COMPATIBILITY_CLASS */:
      return "GL_VIEW_COMPATIBILITY_CLASS (0x82B6)";
    case 0x82B7 /* GL_FULL_SUPPORT */:
      return "GL_FULL_SUPPORT (0x82B7)";
    case 0x82B8 /* GL_CAVEAT_SUPPORT */:
      return "GL_CAVEAT_SUPPORT (0x82B8)";
    case 0x82B9 /* GL_IMAGE_CLASS_4_X_32 */:
      return "GL_IMAGE_CLASS_4_X_32 (0x82B9)";
    case 0x82BA /* GL_IMAGE_CLASS_2_X_32 */:
      return "GL_IMAGE_CLASS_2_X_32 (0x82BA)";
    case 0x82BB /* GL_IMAGE_CLASS_1_X_32 */:
      return "GL_IMAGE_CLASS_1_X_32 (0x82BB)";
    case 0x82BC /* GL_IMAGE_CLASS_4_X_16 */:
      return "GL_IMAGE_CLASS_4_X_16 (0x82BC)";
    case 0x82BD /* GL_IMAGE_CLASS_2_X_16 */:
      return "GL_IMAGE_CLASS_2_X_16 (0x82BD)";
    case 0x82BE /* GL_IMAGE_CLASS_1_X_16 */:
      return "GL_IMAGE_CLASS_1_X_16 (0x82BE)";
    case 0x82BF /* GL_IMAGE_CLASS_4_X_8 */:
      return "GL_IMAGE_CLASS_4_X_8 (0x82BF)";
    case 0x82C0 /* GL_IMAGE_CLASS_2_X_8 */:
      return "GL_IMAGE_CLASS_2_X_8 (0x82C0)";
    case 0x82C1 /* GL_IMAGE_CLASS_1_X_8 */:
      return "GL_IMAGE_CLASS_1_X_8 (0x82C1)";
    case 0x82C2 /* GL_IMAGE_CLASS_11_11_10 */:
      return "GL_IMAGE_CLASS_11_11_10 (0x82C2)";
    case 0x82C3 /* GL_IMAGE_CLASS_10_10_10_2 */:
      return "GL_IMAGE_CLASS_10_10_10_2 (0x82C3)";
    case 0x82C4 /* GL_VIEW_CLASS_128_BITS */:
      return "GL_VIEW_CLASS_128_BITS (0x82C4)";
    case 0x82C5 /* GL_VIEW_CLASS_96_BITS */:
      return "GL_VIEW_CLASS_96_BITS (0x82C5)";
    case 0x82C6 /* GL_VIEW_CLASS_64_BITS */:
      return "GL_VIEW_CLASS_64_BITS (0x82C6)";
    case 0x82C7 /* GL_VIEW_CLASS_48_BITS */:
      return "GL_VIEW_CLASS_48_BITS (0x82C7)";
    case 0x82C8 /* GL_VIEW_CLASS_32_BITS */:
      return "GL_VIEW_CLASS_32_BITS (0x82C8)";
    case 0x82C9 /* GL_VIEW_CLASS_24_BITS */:
      return "GL_VIEW_CLASS_24_BITS (0x82C9)";
    case 0x82CA /* GL_VIEW_CLASS_16_BITS */:
      return "GL_VIEW_CLASS_16_BITS (0x82CA)";
    case 0x82CB /* GL_VIEW_CLASS_8_BITS */:
      return "GL_VIEW_CLASS_8_BITS (0x82CB)";
    case 0x82CC /* GL_VIEW_CLASS_S3TC_DXT1_RGB */:
      return "GL_VIEW_CLASS_S3TC_DXT1_RGB (0x82CC)";
    case 0x82CD /* GL_VIEW_CLASS_S3TC_DXT1_RGBA */:
      return "GL_VIEW_CLASS_S3TC_DXT1_RGBA (0x82CD)";
    case 0x82CE /* GL_VIEW_CLASS_S3TC_DXT3_RGBA */:
      return "GL_VIEW_CLASS_S3TC_DXT3_RGBA (0x82CE)";
    case 0x82CF /* GL_VIEW_CLASS_S3TC_DXT5_RGBA */:
      return "GL_VIEW_CLASS_S3TC_DXT5_RGBA (0x82CF)";
    case 0x82D0 /* GL_VIEW_CLASS_RGTC1_RED */:
      return "GL_VIEW_CLASS_RGTC1_RED (0x82D0)";
    case 0x82D1 /* GL_VIEW_CLASS_RGTC2_RG */:
      return "GL_VIEW_CLASS_RGTC2_RG (0x82D1)";
    case 0x82D2 /* GL_VIEW_CLASS_BPTC_UNORM */:
      return "GL_VIEW_CLASS_BPTC_UNORM (0x82D2)";
    case 0x82D3 /* GL_VIEW_CLASS_BPTC_FLOAT */:
      return "GL_VIEW_CLASS_BPTC_FLOAT (0x82D3)";
    // case 0x84F5 /* GL_TEXTURE_RECTANGLE */: return "GL_TEXTURE_RECTANGLE
    // (0x84F5)"; case 0x8C18 /* GL_TEXTURE_1D_ARRAY */: return
    // "GL_TEXTURE_1D_ARRAY (0x8C18)"; case 0x8C1A /* GL_TEXTURE_2D_ARRAY */:
    // return "GL_TEXTURE_2D_ARRAY (0x8C1A)"; case 0x8C2A /* GL_TEXTURE_BUFFER
    // */: return "GL_TEXTURE_BUFFER (0x8C2A)"; case 0x8D41 /* GL_RENDERBUFFER
    // */: return "GL_RENDERBUFFER (0x8D41)"; case 0x9009 /*
    // GL_TEXTURE_CUBE_MAP_ARRAY */: return "GL_TEXTURE_CUBE_MAP_ARRAY (0x9009)";
    case 0x9100 /* GL_TEXTURE_2D_MULTISAMPLE */:
      return "GL_TEXTURE_2D_MULTISAMPLE (0x9100)";
    case 0x9102 /* GL_TEXTURE_2D_MULTISAMPLE_ARRAY */:
      return "GL_TEXTURE_2D_MULTISAMPLE_ARRAY (0x9102)";
      // case 0x9380 /* GL_NUM_SAMPLE_COUNTS */: return "GL_NUM_SAMPLE_COUNTS
      // (0x9380)";
      /* ----------------------- GL_ARB_invalidate_subdata
       * ----------------------- */

      /* ---------------------- GL_ARB_map_buffer_alignment
       * ---------------------- */

    case 0x90BC /* GL_MIN_MAP_BUFFER_ALIGNMENT */:
      return "GL_MIN_MAP_BUFFER_ALIGNMENT (0x90BC)";

      /* ------------------------ GL_ARB_map_buffer_range
       * ------------------------ */

      // case 0x0001 /* GL_MAP_READ_BIT */: return "GL_MAP_READ_BIT (0x0001)";
      // case 0x0002 /* GL_MAP_WRITE_BIT */: return "GL_MAP_WRITE_BIT (0x0002)";
      // case 0x0004 /* GL_MAP_INVALIDATE_RANGE_BIT */: return
      // "GL_MAP_INVALIDATE_RANGE_BIT (0x0004)"; case 0x0008 /*
      // GL_MAP_INVALIDATE_BUFFER_BIT */: return "GL_MAP_INVALIDATE_BUFFER_BIT
      // (0x0008)"; case 0x0010 /* GL_MAP_FLUSH_EXPLICIT_BIT */: return
      // "GL_MAP_FLUSH_EXPLICIT_BIT (0x0010)"; case 0x0020 /*
      // GL_MAP_UNSYNCHRONIZED_BIT */: return "GL_MAP_UNSYNCHRONIZED_BIT
      // (0x0020)";

      /* ------------------------- GL_ARB_matrix_palette
       * ------------------------- */

    case 0x8840 /* GL_MATRIX_PALETTE_ARB */:
      return "GL_MATRIX_PALETTE_ARB (0x8840)";
    case 0x8841 /* GL_MAX_MATRIX_PALETTE_STACK_DEPTH_ARB */:
      return "GL_MAX_MATRIX_PALETTE_STACK_DEPTH_ARB (0x8841)";
    case 0x8842 /* GL_MAX_PALETTE_MATRICES_ARB */:
      return "GL_MAX_PALETTE_MATRICES_ARB (0x8842)";
    case 0x8843 /* GL_CURRENT_PALETTE_MATRIX_ARB */:
      return "GL_CURRENT_PALETTE_MATRIX_ARB (0x8843)";
    case 0x8844 /* GL_MATRIX_INDEX_ARRAY_ARB */:
      return "GL_MATRIX_INDEX_ARRAY_ARB (0x8844)";
    case 0x8845 /* GL_CURRENT_MATRIX_INDEX_ARB */:
      return "GL_CURRENT_MATRIX_INDEX_ARB (0x8845)";
    case 0x8846 /* GL_MATRIX_INDEX_ARRAY_SIZE_ARB */:
      return "GL_MATRIX_INDEX_ARRAY_SIZE_ARB (0x8846)";
    case 0x8847 /* GL_MATRIX_INDEX_ARRAY_TYPE_ARB */:
      return "GL_MATRIX_INDEX_ARRAY_TYPE_ARB (0x8847)";
    case 0x8848 /* GL_MATRIX_INDEX_ARRAY_STRIDE_ARB */:
      return "GL_MATRIX_INDEX_ARRAY_STRIDE_ARB (0x8848)";
    case 0x8849 /* GL_MATRIX_INDEX_ARRAY_POINTER_ARB */:
      return "GL_MATRIX_INDEX_ARRAY_POINTER_ARB (0x8849)";

      /* ----------------------- GL_ARB_multi_draw_indirect
       * ---------------------- */

      /* --------------------------- GL_ARB_multisample
       * -------------------------- */

      // case 0x809D /* GL_MULTISAMPLE_ARB */: return "GL_MULTISAMPLE_ARB
      // (0x809D)"; case 0x809E /* GL_SAMPLE_ALPHA_TO_COVERAGE_ARB */: return
      // "GL_SAMPLE_ALPHA_TO_COVERAGE_ARB (0x809E)"; case 0x809F /*
      // GL_SAMPLE_ALPHA_TO_ONE_ARB */: return "GL_SAMPLE_ALPHA_TO_ONE_ARB
      // (0x809F)"; case 0x80A0 /* GL_SAMPLE_COVERAGE_ARB */: return
      // "GL_SAMPLE_COVERAGE_ARB (0x80A0)"; case 0x80A8 /* GL_SAMPLE_BUFFERS_ARB
      // */: return "GL_SAMPLE_BUFFERS_ARB (0x80A8)"; case 0x80A9 /*
      // GL_SAMPLES_ARB */: return "GL_SAMPLES_ARB (0x80A9)"; case 0x80AA /*
      // GL_SAMPLE_COVERAGE_VALUE_ARB */: return "GL_SAMPLE_COVERAGE_VALUE_ARB
      // (0x80AA)"; case 0x80AB /* GL_SAMPLE_COVERAGE_INVERT_ARB */: return
      // "GL_SAMPLE_COVERAGE_INVERT_ARB (0x80AB)"; case 0x20000000 /*
      // GL_MULTISAMPLE_BIT_ARB */: return "GL_MULTISAMPLE_BIT_ARB
      // (0x20000000)";/* -------------------------- GL_ARB_multitexture
      // -------------------------- */

      // case 0x84C0 /* GL_TEXTURE0_ARB */: return "GL_TEXTURE0_ARB (0x84C0)";
      // case 0x84C1 /* GL_TEXTURE1_ARB */: return "GL_TEXTURE1_ARB (0x84C1)";
      // case 0x84C2 /* GL_TEXTURE2_ARB */: return "GL_TEXTURE2_ARB (0x84C2)";
      // case 0x84C3 /* GL_TEXTURE3_ARB */: return "GL_TEXTURE3_ARB (0x84C3)";
      // case 0x84C4 /* GL_TEXTURE4_ARB */: return "GL_TEXTURE4_ARB (0x84C4)";
      // case 0x84C5 /* GL_TEXTURE5_ARB */: return "GL_TEXTURE5_ARB (0x84C5)";
      // case 0x84C6 /* GL_TEXTURE6_ARB */: return "GL_TEXTURE6_ARB (0x84C6)";
      // case 0x84C7 /* GL_TEXTURE7_ARB */: return "GL_TEXTURE7_ARB (0x84C7)";
      // case 0x84C8 /* GL_TEXTURE8_ARB */: return "GL_TEXTURE8_ARB (0x84C8)";
      // case 0x84C9 /* GL_TEXTURE9_ARB */: return "GL_TEXTURE9_ARB (0x84C9)";
      // case 0x84CA /* GL_TEXTURE10_ARB */: return "GL_TEXTURE10_ARB (0x84CA)";
      // case 0x84CB /* GL_TEXTURE11_ARB */: return "GL_TEXTURE11_ARB (0x84CB)";
      // case 0x84CC /* GL_TEXTURE12_ARB */: return "GL_TEXTURE12_ARB (0x84CC)";
      // case 0x84CD /* GL_TEXTURE13_ARB */: return "GL_TEXTURE13_ARB (0x84CD)";
      // case 0x84CE /* GL_TEXTURE14_ARB */: return "GL_TEXTURE14_ARB (0x84CE)";
      // case 0x84CF /* GL_TEXTURE15_ARB */: return "GL_TEXTURE15_ARB (0x84CF)";
      // case 0x84D0 /* GL_TEXTURE16_ARB */: return "GL_TEXTURE16_ARB (0x84D0)";
      // case 0x84D1 /* GL_TEXTURE17_ARB */: return "GL_TEXTURE17_ARB (0x84D1)";
      // case 0x84D2 /* GL_TEXTURE18_ARB */: return "GL_TEXTURE18_ARB (0x84D2)";
      // case 0x84D3 /* GL_TEXTURE19_ARB */: return "GL_TEXTURE19_ARB (0x84D3)";
      // case 0x84D4 /* GL_TEXTURE20_ARB */: return "GL_TEXTURE20_ARB (0x84D4)";
      // case 0x84D5 /* GL_TEXTURE21_ARB */: return "GL_TEXTURE21_ARB (0x84D5)";
      // case 0x84D6 /* GL_TEXTURE22_ARB */: return "GL_TEXTURE22_ARB (0x84D6)";
      // case 0x84D7 /* GL_TEXTURE23_ARB */: return "GL_TEXTURE23_ARB (0x84D7)";
      // case 0x84D8 /* GL_TEXTURE24_ARB */: return "GL_TEXTURE24_ARB (0x84D8)";
      // case 0x84D9 /* GL_TEXTURE25_ARB */: return "GL_TEXTURE25_ARB (0x84D9)";
      // case 0x84DA /* GL_TEXTURE26_ARB */: return "GL_TEXTURE26_ARB (0x84DA)";
      // case 0x84DB /* GL_TEXTURE27_ARB */: return "GL_TEXTURE27_ARB (0x84DB)";
      // case 0x84DC /* GL_TEXTURE28_ARB */: return "GL_TEXTURE28_ARB (0x84DC)";
      // case 0x84DD /* GL_TEXTURE29_ARB */: return "GL_TEXTURE29_ARB (0x84DD)";
      // case 0x84DE /* GL_TEXTURE30_ARB */: return "GL_TEXTURE30_ARB (0x84DE)";
      // case 0x84DF /* GL_TEXTURE31_ARB */: return "GL_TEXTURE31_ARB (0x84DF)";
      // case 0x84E0 /* GL_ACTIVE_TEXTURE_ARB */: return "GL_ACTIVE_TEXTURE_ARB
      // (0x84E0)"; case 0x84E1 /* GL_CLIENT_ACTIVE_TEXTURE_ARB */: return
      // "GL_CLIENT_ACTIVE_TEXTURE_ARB (0x84E1)"; case 0x84E2 /*
      // GL_MAX_TEXTURE_UNITS_ARB */: return "GL_MAX_TEXTURE_UNITS_ARB (0x84E2)";

      /* ------------------------- GL_ARB_occlusion_query
       * ------------------------ */

      // case 0x8864 /* GL_QUERY_COUNTER_BITS_ARB */: return
      // "GL_QUERY_COUNTER_BITS_ARB (0x8864)"; case 0x8865 /*
      // GL_CURRENT_QUERY_ARB */: return "GL_CURRENT_QUERY_ARB (0x8865)"; case
      // 0x8866 /* GL_QUERY_RESULT_ARB */: return "GL_QUERY_RESULT_ARB (0x8866)";
      // case 0x8867 /* GL_QUERY_RESULT_AVAILABLE_ARB */: return
      // "GL_QUERY_RESULT_AVAILABLE_ARB (0x8867)"; case 0x8914 /*
      // GL_SAMPLES_PASSED_ARB */: return "GL_SAMPLES_PASSED_ARB (0x8914)";

      /* ------------------------ GL_ARB_occlusion_query2
       * ------------------------ */

    case 0x8C2F /* GL_ANY_SAMPLES_PASSED */:
      return "GL_ANY_SAMPLES_PASSED (0x8C2F)";

      /* ----------------------- GL_ARB_pixel_buffer_object
       * ---------------------- */

      // case 0x88EB /* GL_PIXEL_PACK_BUFFER_ARB */: return
      // "GL_PIXEL_PACK_BUFFER_ARB (0x88EB)"; case 0x88EC /*
      // GL_PIXEL_UNPACK_BUFFER_ARB */: return "GL_PIXEL_UNPACK_BUFFER_ARB
      // (0x88EC)"; case 0x88ED /* GL_PIXEL_PACK_BUFFER_BINDING_ARB */: return
      // "GL_PIXEL_PACK_BUFFER_BINDING_ARB (0x88ED)"; case 0x88EF /*
      // GL_PIXEL_UNPACK_BUFFER_BINDING_ARB */: return
      // "GL_PIXEL_UNPACK_BUFFER_BINDING_ARB (0x88EF)";

      /* ------------------------ GL_ARB_point_parameters
       * ------------------------ */

      // case 0x8126 /* GL_POINT_SIZE_MIN_ARB */: return "GL_POINT_SIZE_MIN_ARB
      // (0x8126)"; case 0x8127 /* GL_POINT_SIZE_MAX_ARB */: return
      // "GL_POINT_SIZE_MAX_ARB (0x8127)"; case 0x8128 /*
      // GL_POINT_FADE_THRESHOLD_SIZE_ARB */: return
      // "GL_POINT_FADE_THRESHOLD_SIZE_ARB (0x8128)"; case 0x8129 /*
      // GL_POINT_DISTANCE_ATTENUATION_ARB */: return
      // "GL_POINT_DISTANCE_ATTENUATION_ARB (0x8129)";

      /* -------------------------- GL_ARB_point_sprite
       * -------------------------- */

      // case 0x8861 /* GL_POINT_SPRITE_ARB */: return "GL_POINT_SPRITE_ARB
      // (0x8861)"; case 0x8862 /* GL_COORD_REPLACE_ARB */: return
      // "GL_COORD_REPLACE_ARB (0x8862)";

      /* --------------------- GL_ARB_program_interface_query
       * -------------------- */

    case 0x92E1 /* GL_UNIFORM */:
      return "GL_UNIFORM (0x92E1)";
    case 0x92E2 /* GL_UNIFORM_BLOCK */:
      return "GL_UNIFORM_BLOCK (0x92E2)";
    case 0x92E3 /* GL_PROGRAM_INPUT */:
      return "GL_PROGRAM_INPUT (0x92E3)";
    case 0x92E4 /* GL_PROGRAM_OUTPUT */:
      return "GL_PROGRAM_OUTPUT (0x92E4)";
    case 0x92E5 /* GL_BUFFER_VARIABLE */:
      return "GL_BUFFER_VARIABLE (0x92E5)";
    case 0x92E6 /* GL_SHADER_STORAGE_BLOCK */:
      return "GL_SHADER_STORAGE_BLOCK (0x92E6)";
    case 0x92E7 /* GL_IS_PER_PATCH */:
      return "GL_IS_PER_PATCH (0x92E7)";
    case 0x92E8 /* GL_VERTEX_SUBROUTINE */:
      return "GL_VERTEX_SUBROUTINE (0x92E8)";
    case 0x92E9 /* GL_TESS_CONTROL_SUBROUTINE */:
      return "GL_TESS_CONTROL_SUBROUTINE (0x92E9)";
    case 0x92EA /* GL_TESS_EVALUATION_SUBROUTINE */:
      return "GL_TESS_EVALUATION_SUBROUTINE (0x92EA)";
    case 0x92EB /* GL_GEOMETRY_SUBROUTINE */:
      return "GL_GEOMETRY_SUBROUTINE (0x92EB)";
    case 0x92EC /* GL_FRAGMENT_SUBROUTINE */:
      return "GL_FRAGMENT_SUBROUTINE (0x92EC)";
    case 0x92ED /* GL_COMPUTE_SUBROUTINE */:
      return "GL_COMPUTE_SUBROUTINE (0x92ED)";
    case 0x92EE /* GL_VERTEX_SUBROUTINE_UNIFORM */:
      return "GL_VERTEX_SUBROUTINE_UNIFORM (0x92EE)";
    case 0x92EF /* GL_TESS_CONTROL_SUBROUTINE_UNIFORM */:
      return "GL_TESS_CONTROL_SUBROUTINE_UNIFORM (0x92EF)";
    case 0x92F0 /* GL_TESS_EVALUATION_SUBROUTINE_UNIFORM */:
      return "GL_TESS_EVALUATION_SUBROUTINE_UNIFORM (0x92F0)";
    case 0x92F1 /* GL_GEOMETRY_SUBROUTINE_UNIFORM */:
      return "GL_GEOMETRY_SUBROUTINE_UNIFORM (0x92F1)";
    case 0x92F2 /* GL_FRAGMENT_SUBROUTINE_UNIFORM */:
      return "GL_FRAGMENT_SUBROUTINE_UNIFORM (0x92F2)";
    case 0x92F3 /* GL_COMPUTE_SUBROUTINE_UNIFORM */:
      return "GL_COMPUTE_SUBROUTINE_UNIFORM (0x92F3)";
    case 0x92F4 /* GL_TRANSFORM_FEEDBACK_VARYING */:
      return "GL_TRANSFORM_FEEDBACK_VARYING (0x92F4)";
    case 0x92F5 /* GL_ACTIVE_RESOURCES */:
      return "GL_ACTIVE_RESOURCES (0x92F5)";
    case 0x92F6 /* GL_MAX_NAME_LENGTH */:
      return "GL_MAX_NAME_LENGTH (0x92F6)";
    case 0x92F7 /* GL_MAX_NUM_ACTIVE_VARIABLES */:
      return "GL_MAX_NUM_ACTIVE_VARIABLES (0x92F7)";
    case 0x92F8 /* GL_MAX_NUM_COMPATIBLE_SUBROUTINES */:
      return "GL_MAX_NUM_COMPATIBLE_SUBROUTINES (0x92F8)";
    case 0x92F9 /* GL_NAME_LENGTH */:
      return "GL_NAME_LENGTH (0x92F9)";
    case 0x92FA /* GL_TYPE */:
      return "GL_TYPE (0x92FA)";
    case 0x92FB /* GL_ARRAY_SIZE */:
      return "GL_ARRAY_SIZE (0x92FB)";
    case 0x92FC /* GL_OFFSET */:
      return "GL_OFFSET (0x92FC)";
    case 0x92FD /* GL_BLOCK_INDEX */:
      return "GL_BLOCK_INDEX (0x92FD)";
    case 0x92FE /* GL_ARRAY_STRIDE */:
      return "GL_ARRAY_STRIDE (0x92FE)";
    case 0x92FF /* GL_MATRIX_STRIDE */:
      return "GL_MATRIX_STRIDE (0x92FF)";
    case 0x9300 /* GL_IS_ROW_MAJOR */:
      return "GL_IS_ROW_MAJOR (0x9300)";
    case 0x9301 /* GL_ATOMIC_COUNTER_BUFFER_INDEX */:
      return "GL_ATOMIC_COUNTER_BUFFER_INDEX (0x9301)";
    case 0x9302 /* GL_BUFFER_BINDING */:
      return "GL_BUFFER_BINDING (0x9302)";
    case 0x9303 /* GL_BUFFER_DATA_SIZE */:
      return "GL_BUFFER_DATA_SIZE (0x9303)";
    case 0x9304 /* GL_NUM_ACTIVE_VARIABLES */:
      return "GL_NUM_ACTIVE_VARIABLES (0x9304)";
    case 0x9305 /* GL_ACTIVE_VARIABLES */:
      return "GL_ACTIVE_VARIABLES (0x9305)";
    case 0x9306 /* GL_REFERENCED_BY_VERTEX_SHADER */:
      return "GL_REFERENCED_BY_VERTEX_SHADER (0x9306)";
    case 0x9307 /* GL_REFERENCED_BY_TESS_CONTROL_SHADER */:
      return "GL_REFERENCED_BY_TESS_CONTROL_SHADER (0x9307)";
    case 0x9308 /* GL_REFERENCED_BY_TESS_EVALUATION_SHADER */:
      return "GL_REFERENCED_BY_TESS_EVALUATION_SHADER (0x9308)";
    case 0x9309 /* GL_REFERENCED_BY_GEOMETRY_SHADER */:
      return "GL_REFERENCED_BY_GEOMETRY_SHADER (0x9309)";
    case 0x930A /* GL_REFERENCED_BY_FRAGMENT_SHADER */:
      return "GL_REFERENCED_BY_FRAGMENT_SHADER (0x930A)";
    case 0x930B /* GL_REFERENCED_BY_COMPUTE_SHADER */:
      return "GL_REFERENCED_BY_COMPUTE_SHADER (0x930B)";
    case 0x930C /* GL_TOP_LEVEL_ARRAY_SIZE */:
      return "GL_TOP_LEVEL_ARRAY_SIZE (0x930C)";
    case 0x930D /* GL_TOP_LEVEL_ARRAY_STRIDE */:
      return "GL_TOP_LEVEL_ARRAY_STRIDE (0x930D)";
    case 0x930E /* GL_LOCATION */:
      return "GL_LOCATION (0x930E)";
    case 0x930F /* GL_LOCATION_INDEX */:
      return "GL_LOCATION_INDEX (0x930F)"; /* ------------------------
                                              GL_ARB_provoking_vertex
                                              ------------------------ */

    case 0x8E4C /* GL_QUADS_FOLLOW_PROVOKING_VERTEX_CONVENTION */:
      return "GL_QUADS_FOLLOW_PROVOKING_VERTEX_CONVENTION (0x8E4C)";
    case 0x8E4D /* GL_FIRST_VERTEX_CONVENTION */:
      return "GL_FIRST_VERTEX_CONVENTION (0x8E4D)";
    case 0x8E4E /* GL_LAST_VERTEX_CONVENTION */:
      return "GL_LAST_VERTEX_CONVENTION (0x8E4E)";
    case 0x8E4F /* GL_PROVOKING_VERTEX */:
      return "GL_PROVOKING_VERTEX (0x8E4F)"; /* ------------------
                                                GL_ARB_robust_buffer_access_behavior
                                                ----------------- */

    /* --------------------------- GL_ARB_robustness ---------------------------
     */

    // case 0x00000004 /* GL_CONTEXT_FLAG_ROBUST_ACCESS_BIT_ARB */: return
    // "GL_CONTEXT_FLAG_ROBUST_ACCESS_BIT_ARB (0x00000004)";
    case 0x8252 /* GL_LOSE_CONTEXT_ON_RESET_ARB */:
      return "GL_LOSE_CONTEXT_ON_RESET_ARB (0x8252)";
    case 0x8253 /* GL_GUILTY_CONTEXT_RESET_ARB */:
      return "GL_GUILTY_CONTEXT_RESET_ARB (0x8253)";
    case 0x8254 /* GL_INNOCENT_CONTEXT_RESET_ARB */:
      return "GL_INNOCENT_CONTEXT_RESET_ARB (0x8254)";
    case 0x8255 /* GL_UNKNOWN_CONTEXT_RESET_ARB */:
      return "GL_UNKNOWN_CONTEXT_RESET_ARB (0x8255)";
    case 0x8256 /* GL_RESET_NOTIFICATION_STRATEGY_ARB */:
      return "GL_RESET_NOTIFICATION_STRATEGY_ARB (0x8256)";
    case 0x8261 /* GL_NO_RESET_NOTIFICATION_ARB */:
      return "GL_NO_RESET_NOTIFICATION_ARB (0x8261)";

      /* ---------------- GL_ARB_robustness_application_isolation
       * ---------------- */

      /* ---------------- GL_ARB_robustness_share_group_isolation
       * ---------------- */

      /* ------------------------- GL_ARB_sample_shading
       * ------------------------- */

      // case 0x8C36 /* GL_SAMPLE_SHADING_ARB */: return "GL_SAMPLE_SHADING_ARB
      // (0x8C36)"; case 0x8C37 /* GL_MIN_SAMPLE_SHADING_VALUE_ARB */: return
      // "GL_MIN_SAMPLE_SHADING_VALUE_ARB (0x8C37)";
      /* ------------------------- GL_ARB_sampler_objects
       * ------------------------ */

    case 0x8919 /* GL_SAMPLER_BINDING */:
      return "GL_SAMPLER_BINDING (0x8919)";

    /* ------------------------ GL_ARB_seamless_cube_map -----------------------
     */

    // case 0x884F /* GL_TEXTURE_CUBE_MAP_SEAMLESS */: return
    // "GL_TEXTURE_CUBE_MAP_SEAMLESS (0x884F)";

    /* --------------------- GL_ARB_separate_shader_objects --------------------
     */

    // case 0x00000001 /* GL_VERTEX_SHADER_BIT */: return "GL_VERTEX_SHADER_BIT
    // (0x00000001)"; case 0x00000002 /* GL_FRAGMENT_SHADER_BIT */: return
    // "GL_FRAGMENT_SHADER_BIT (0x00000002)"; case 0x00000004 /*
    // GL_GEOMETRY_SHADER_BIT */: return "GL_GEOMETRY_SHADER_BIT (0x00000004)";
    // case 0x00000008 /* GL_TESS_CONTROL_SHADER_BIT */: return
    // "GL_TESS_CONTROL_SHADER_BIT (0x00000008)"; case 0x00000010 /*
    // GL_TESS_EVALUATION_SHADER_BIT */: return "GL_TESS_EVALUATION_SHADER_BIT
    // (0x00000010)";
    case 0x8258 /* GL_PROGRAM_SEPARABLE */:
      return "GL_PROGRAM_SEPARABLE (0x8258)";
    case 0x8259 /* GL_ACTIVE_PROGRAM */:
      return "GL_ACTIVE_PROGRAM (0x8259)";
    case 0x825A /* GL_PROGRAM_PIPELINE_BINDING */:
      return "GL_PROGRAM_PIPELINE_BINDING (0x825A)";
      // case 0xFFFFFFFF /* GL_ALL_SHADER_BITS */: return "GL_ALL_SHADER_BITS
      // (0xFFFFFFFF)";

      /* --------------------- GL_ARB_shader_atomic_counters
       * --------------------- */

    case 0x92C0 /* GL_ATOMIC_COUNTER_BUFFER */:
      return "GL_ATOMIC_COUNTER_BUFFER (0x92C0)";
    case 0x92C1 /* GL_ATOMIC_COUNTER_BUFFER_BINDING */:
      return "GL_ATOMIC_COUNTER_BUFFER_BINDING (0x92C1)";
    case 0x92C2 /* GL_ATOMIC_COUNTER_BUFFER_START */:
      return "GL_ATOMIC_COUNTER_BUFFER_START (0x92C2)";
    case 0x92C3 /* GL_ATOMIC_COUNTER_BUFFER_SIZE */:
      return "GL_ATOMIC_COUNTER_BUFFER_SIZE (0x92C3)";
    case 0x92C4 /* GL_ATOMIC_COUNTER_BUFFER_DATA_SIZE */:
      return "GL_ATOMIC_COUNTER_BUFFER_DATA_SIZE (0x92C4)";
    case 0x92C5 /* GL_ATOMIC_COUNTER_BUFFER_ACTIVE_ATOMIC_COUNTERS */:
      return "GL_ATOMIC_COUNTER_BUFFER_ACTIVE_ATOMIC_COUNTERS (0x92C5)";
    case 0x92C6 /* GL_ATOMIC_COUNTER_BUFFER_ACTIVE_ATOMIC_COUNTER_INDICES */:
      return "GL_ATOMIC_COUNTER_BUFFER_ACTIVE_ATOMIC_COUNTER_INDICES (0x92C6)";
    case 0x92C7 /* GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_VERTEX_SHADER */:
      return "GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_VERTEX_SHADER (0x92C7)";
    case 0x92C8 /* GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_TESS_CONTROL_SHADER */:
      return "GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_TESS_CONTROL_SHADER "
             "(0x92C8)";
    case 0x92C9 /* GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_TESS_EVALUATION_SHADER */:
      return "GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_TESS_EVALUATION_SHADER "
             "(0x92C9)";
    case 0x92CA /* GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_GEOMETRY_SHADER */:
      return "GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_GEOMETRY_SHADER (0x92CA)";
    case 0x92CB /* GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_FRAGMENT_SHADER */:
      return "GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_FRAGMENT_SHADER (0x92CB)";
    case 0x92CC /* GL_MAX_VERTEX_ATOMIC_COUNTER_BUFFERS */:
      return "GL_MAX_VERTEX_ATOMIC_COUNTER_BUFFERS (0x92CC)";
    case 0x92CD /* GL_MAX_TESS_CONTROL_ATOMIC_COUNTER_BUFFERS */:
      return "GL_MAX_TESS_CONTROL_ATOMIC_COUNTER_BUFFERS (0x92CD)";
    case 0x92CE /* GL_MAX_TESS_EVALUATION_ATOMIC_COUNTER_BUFFERS */:
      return "GL_MAX_TESS_EVALUATION_ATOMIC_COUNTER_BUFFERS (0x92CE)";
    case 0x92CF /* GL_MAX_GEOMETRY_ATOMIC_COUNTER_BUFFERS */:
      return "GL_MAX_GEOMETRY_ATOMIC_COUNTER_BUFFERS (0x92CF)";
    case 0x92D0 /* GL_MAX_FRAGMENT_ATOMIC_COUNTER_BUFFERS */:
      return "GL_MAX_FRAGMENT_ATOMIC_COUNTER_BUFFERS (0x92D0)";
    case 0x92D1 /* GL_MAX_COMBINED_ATOMIC_COUNTER_BUFFERS */:
      return "GL_MAX_COMBINED_ATOMIC_COUNTER_BUFFERS (0x92D1)";
    case 0x92D2 /* GL_MAX_VERTEX_ATOMIC_COUNTERS */:
      return "GL_MAX_VERTEX_ATOMIC_COUNTERS (0x92D2)";
    case 0x92D3 /* GL_MAX_TESS_CONTROL_ATOMIC_COUNTERS */:
      return "GL_MAX_TESS_CONTROL_ATOMIC_COUNTERS (0x92D3)";
    case 0x92D4 /* GL_MAX_TESS_EVALUATION_ATOMIC_COUNTERS */:
      return "GL_MAX_TESS_EVALUATION_ATOMIC_COUNTERS (0x92D4)";
    case 0x92D5 /* GL_MAX_GEOMETRY_ATOMIC_COUNTERS */:
      return "GL_MAX_GEOMETRY_ATOMIC_COUNTERS (0x92D5)";
    case 0x92D6 /* GL_MAX_FRAGMENT_ATOMIC_COUNTERS */:
      return "GL_MAX_FRAGMENT_ATOMIC_COUNTERS (0x92D6)";
    case 0x92D7 /* GL_MAX_COMBINED_ATOMIC_COUNTERS */:
      return "GL_MAX_COMBINED_ATOMIC_COUNTERS (0x92D7)";
    case 0x92D8 /* GL_MAX_ATOMIC_COUNTER_BUFFER_SIZE */:
      return "GL_MAX_ATOMIC_COUNTER_BUFFER_SIZE (0x92D8)";
    case 0x92D9 /* GL_ACTIVE_ATOMIC_COUNTER_BUFFERS */:
      return "GL_ACTIVE_ATOMIC_COUNTER_BUFFERS (0x92D9)";
    case 0x92DA /* GL_UNIFORM_ATOMIC_COUNTER_BUFFER_INDEX */:
      return "GL_UNIFORM_ATOMIC_COUNTER_BUFFER_INDEX (0x92DA)";
    case 0x92DB /* GL_UNSIGNED_INT_ATOMIC_COUNTER */:
      return "GL_UNSIGNED_INT_ATOMIC_COUNTER (0x92DB)";
    case 0x92DC /* GL_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS */:
      return "GL_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS (0x92DC)"; /* -----------------------
                                                                  GL_ARB_shader_bit_encoding
                                                                  ----------------------
                                                                */

    /* --------------------- GL_ARB_shader_image_load_store --------------------
     */

    // case 0x00000001 /* GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT */: return
    // "GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT (0x00000001)"; case 0x00000002 /*
    // GL_ELEMENT_ARRAY_BARRIER_BIT */: return "GL_ELEMENT_ARRAY_BARRIER_BIT
    // (0x00000002)"; case 0x00000004 /* GL_UNIFORM_BARRIER_BIT */: return
    // "GL_UNIFORM_BARRIER_BIT (0x00000004)"; case 0x00000008 /*
    // GL_TEXTURE_FETCH_BARRIER_BIT */: return "GL_TEXTURE_FETCH_BARRIER_BIT
    // (0x00000008)"; case 0x00000020 /* GL_SHADER_IMAGE_ACCESS_BARRIER_BIT */:
    // return "GL_SHADER_IMAGE_ACCESS_BARRIER_BIT (0x00000020)"; case 0x00000040
    // /* GL_COMMAND_BARRIER_BIT */: return "GL_COMMAND_BARRIER_BIT
    // (0x00000040)"; case 0x00000080 /* GL_PIXEL_BUFFER_BARRIER_BIT */: return
    // "GL_PIXEL_BUFFER_BARRIER_BIT (0x00000080)"; case 0x00000100 /*
    // GL_TEXTURE_UPDATE_BARRIER_BIT */: return "GL_TEXTURE_UPDATE_BARRIER_BIT
    // (0x00000100)"; case 0x00000200 /* GL_BUFFER_UPDATE_BARRIER_BIT */: return
    // "GL_BUFFER_UPDATE_BARRIER_BIT (0x00000200)"; case 0x00000400 /*
    // GL_FRAMEBUFFER_BARRIER_BIT */: return "GL_FRAMEBUFFER_BARRIER_BIT
    // (0x00000400)"; case 0x00000800 /* GL_TRANSFORM_FEEDBACK_BARRIER_BIT */:
    // return "GL_TRANSFORM_FEEDBACK_BARRIER_BIT (0x00000800)"; case 0x00001000
    // /* GL_ATOMIC_COUNTER_BARRIER_BIT */: return "GL_ATOMIC_COUNTER_BARRIER_BIT
    // (0x00001000)";
    case 0x8F38 /* GL_MAX_IMAGE_UNITS */:
      return "GL_MAX_IMAGE_UNITS (0x8F38)";
    case 0x8F39 /* GL_MAX_COMBINED_IMAGE_UNITS_AND_FRAGMENT_OUTPUTS */:
      return "GL_MAX_COMBINED_IMAGE_UNITS_AND_FRAGMENT_OUTPUTS (0x8F39)";
    case 0x8F3A /* GL_IMAGE_BINDING_NAME */:
      return "GL_IMAGE_BINDING_NAME (0x8F3A)";
    case 0x8F3B /* GL_IMAGE_BINDING_LEVEL */:
      return "GL_IMAGE_BINDING_LEVEL (0x8F3B)";
    case 0x8F3C /* GL_IMAGE_BINDING_LAYERED */:
      return "GL_IMAGE_BINDING_LAYERED (0x8F3C)";
    case 0x8F3D /* GL_IMAGE_BINDING_LAYER */:
      return "GL_IMAGE_BINDING_LAYER (0x8F3D)";
    case 0x8F3E /* GL_IMAGE_BINDING_ACCESS */:
      return "GL_IMAGE_BINDING_ACCESS (0x8F3E)";
    case 0x904C /* GL_IMAGE_1D */:
      return "GL_IMAGE_1D (0x904C)";
    case 0x904D /* GL_IMAGE_2D */:
      return "GL_IMAGE_2D (0x904D)";
    case 0x904E /* GL_IMAGE_3D */:
      return "GL_IMAGE_3D (0x904E)";
    case 0x904F /* GL_IMAGE_2D_RECT */:
      return "GL_IMAGE_2D_RECT (0x904F)";
    case 0x9050 /* GL_IMAGE_CUBE */:
      return "GL_IMAGE_CUBE (0x9050)";
    case 0x9051 /* GL_IMAGE_BUFFER */:
      return "GL_IMAGE_BUFFER (0x9051)";
    case 0x9052 /* GL_IMAGE_1D_ARRAY */:
      return "GL_IMAGE_1D_ARRAY (0x9052)";
    case 0x9053 /* GL_IMAGE_2D_ARRAY */:
      return "GL_IMAGE_2D_ARRAY (0x9053)";
    case 0x9054 /* GL_IMAGE_CUBE_MAP_ARRAY */:
      return "GL_IMAGE_CUBE_MAP_ARRAY (0x9054)";
    case 0x9055 /* GL_IMAGE_2D_MULTISAMPLE */:
      return "GL_IMAGE_2D_MULTISAMPLE (0x9055)";
    case 0x9056 /* GL_IMAGE_2D_MULTISAMPLE_ARRAY */:
      return "GL_IMAGE_2D_MULTISAMPLE_ARRAY (0x9056)";
    case 0x9057 /* GL_INT_IMAGE_1D */:
      return "GL_INT_IMAGE_1D (0x9057)";
    case 0x9058 /* GL_INT_IMAGE_2D */:
      return "GL_INT_IMAGE_2D (0x9058)";
    case 0x9059 /* GL_INT_IMAGE_3D */:
      return "GL_INT_IMAGE_3D (0x9059)";
    case 0x905A /* GL_INT_IMAGE_2D_RECT */:
      return "GL_INT_IMAGE_2D_RECT (0x905A)";
    case 0x905B /* GL_INT_IMAGE_CUBE */:
      return "GL_INT_IMAGE_CUBE (0x905B)";
    case 0x905C /* GL_INT_IMAGE_BUFFER */:
      return "GL_INT_IMAGE_BUFFER (0x905C)";
    case 0x905D /* GL_INT_IMAGE_1D_ARRAY */:
      return "GL_INT_IMAGE_1D_ARRAY (0x905D)";
    case 0x905E /* GL_INT_IMAGE_2D_ARRAY */:
      return "GL_INT_IMAGE_2D_ARRAY (0x905E)";
    case 0x905F /* GL_INT_IMAGE_CUBE_MAP_ARRAY */:
      return "GL_INT_IMAGE_CUBE_MAP_ARRAY (0x905F)";
    case 0x9060 /* GL_INT_IMAGE_2D_MULTISAMPLE */:
      return "GL_INT_IMAGE_2D_MULTISAMPLE (0x9060)";
    case 0x9061 /* GL_INT_IMAGE_2D_MULTISAMPLE_ARRAY */:
      return "GL_INT_IMAGE_2D_MULTISAMPLE_ARRAY (0x9061)";
    case 0x9062 /* GL_UNSIGNED_INT_IMAGE_1D */:
      return "GL_UNSIGNED_INT_IMAGE_1D (0x9062)";
    case 0x9063 /* GL_UNSIGNED_INT_IMAGE_2D */:
      return "GL_UNSIGNED_INT_IMAGE_2D (0x9063)";
    case 0x9064 /* GL_UNSIGNED_INT_IMAGE_3D */:
      return "GL_UNSIGNED_INT_IMAGE_3D (0x9064)";
    case 0x9065 /* GL_UNSIGNED_INT_IMAGE_2D_RECT */:
      return "GL_UNSIGNED_INT_IMAGE_2D_RECT (0x9065)";
    case 0x9066 /* GL_UNSIGNED_INT_IMAGE_CUBE */:
      return "GL_UNSIGNED_INT_IMAGE_CUBE (0x9066)";
    case 0x9067 /* GL_UNSIGNED_INT_IMAGE_BUFFER */:
      return "GL_UNSIGNED_INT_IMAGE_BUFFER (0x9067)";
    case 0x9068 /* GL_UNSIGNED_INT_IMAGE_1D_ARRAY */:
      return "GL_UNSIGNED_INT_IMAGE_1D_ARRAY (0x9068)";
    case 0x9069 /* GL_UNSIGNED_INT_IMAGE_2D_ARRAY */:
      return "GL_UNSIGNED_INT_IMAGE_2D_ARRAY (0x9069)";
    case 0x906A /* GL_UNSIGNED_INT_IMAGE_CUBE_MAP_ARRAY */:
      return "GL_UNSIGNED_INT_IMAGE_CUBE_MAP_ARRAY (0x906A)";
    case 0x906B /* GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE */:
      return "GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE (0x906B)";
    case 0x906C /* GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY */:
      return "GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY (0x906C)";
    case 0x906D /* GL_MAX_IMAGE_SAMPLES */:
      return "GL_MAX_IMAGE_SAMPLES (0x906D)";
    case 0x906E /* GL_IMAGE_BINDING_FORMAT */:
      return "GL_IMAGE_BINDING_FORMAT (0x906E)";
    case 0x90C7 /* GL_IMAGE_FORMAT_COMPATIBILITY_TYPE */:
      return "GL_IMAGE_FORMAT_COMPATIBILITY_TYPE (0x90C7)";
    case 0x90C8 /* GL_IMAGE_FORMAT_COMPATIBILITY_BY_SIZE */:
      return "GL_IMAGE_FORMAT_COMPATIBILITY_BY_SIZE (0x90C8)";
    case 0x90C9 /* GL_IMAGE_FORMAT_COMPATIBILITY_BY_CLASS */:
      return "GL_IMAGE_FORMAT_COMPATIBILITY_BY_CLASS (0x90C9)";
    case 0x90CA /* GL_MAX_VERTEX_IMAGE_UNIFORMS */:
      return "GL_MAX_VERTEX_IMAGE_UNIFORMS (0x90CA)";
    case 0x90CB /* GL_MAX_TESS_CONTROL_IMAGE_UNIFORMS */:
      return "GL_MAX_TESS_CONTROL_IMAGE_UNIFORMS (0x90CB)";
    case 0x90CC /* GL_MAX_TESS_EVALUATION_IMAGE_UNIFORMS */:
      return "GL_MAX_TESS_EVALUATION_IMAGE_UNIFORMS (0x90CC)";
    case 0x90CD /* GL_MAX_GEOMETRY_IMAGE_UNIFORMS */:
      return "GL_MAX_GEOMETRY_IMAGE_UNIFORMS (0x90CD)";
    case 0x90CE /* GL_MAX_FRAGMENT_IMAGE_UNIFORMS */:
      return "GL_MAX_FRAGMENT_IMAGE_UNIFORMS (0x90CE)";
    case 0x90CF /* GL_MAX_COMBINED_IMAGE_UNIFORMS */:
      return "GL_MAX_COMBINED_IMAGE_UNIFORMS (0x90CF)";
      // case 0xFFFFFFFF /* GL_ALL_BARRIER_BITS */: return "GL_ALL_BARRIER_BITS
      // (0xFFFFFFFF)";

      /* ------------------------ GL_ARB_shader_image_size
       * ----------------------- */

      /* ------------------------- GL_ARB_shader_objects
       * ------------------------- */

    case 0x8B40 /* GL_PROGRAM_OBJECT_ARB */:
      return "GL_PROGRAM_OBJECT_ARB (0x8B40)";
    case 0x8B48 /* GL_SHADER_OBJECT_ARB */:
      return "GL_SHADER_OBJECT_ARB (0x8B48)";
    case 0x8B4E /* GL_OBJECT_TYPE_ARB */:
      return "GL_OBJECT_TYPE_ARB (0x8B4E)";
    // case 0x8B4F /* GL_OBJECT_SUBTYPE_ARB */: return "GL_OBJECT_SUBTYPE_ARB
    // (0x8B4F)"; case 0x8B50 /* GL_FLOAT_VEC2_ARB */: return "GL_FLOAT_VEC2_ARB
    // (0x8B50)"; case 0x8B51 /* GL_FLOAT_VEC3_ARB */: return "GL_FLOAT_VEC3_ARB
    // (0x8B51)"; case 0x8B52 /* GL_FLOAT_VEC4_ARB */: return "GL_FLOAT_VEC4_ARB
    // (0x8B52)"; case 0x8B53 /* GL_INT_VEC2_ARB */: return "GL_INT_VEC2_ARB
    // (0x8B53)"; case 0x8B54 /* GL_INT_VEC3_ARB */: return "GL_INT_VEC3_ARB
    // (0x8B54)"; case 0x8B55 /* GL_INT_VEC4_ARB */: return "GL_INT_VEC4_ARB
    // (0x8B55)"; case 0x8B56 /* GL_BOOL_ARB */: return "GL_BOOL_ARB (0x8B56)";
    // case 0x8B57 /* GL_BOOL_VEC2_ARB */: return "GL_BOOL_VEC2_ARB (0x8B57)";
    // case 0x8B58 /* GL_BOOL_VEC3_ARB */: return "GL_BOOL_VEC3_ARB (0x8B58)";
    // case 0x8B59 /* GL_BOOL_VEC4_ARB */: return "GL_BOOL_VEC4_ARB (0x8B59)";
    // case 0x8B5A /* GL_FLOAT_MAT2_ARB */: return "GL_FLOAT_MAT2_ARB (0x8B5A)";
    // case 0x8B5B /* GL_FLOAT_MAT3_ARB */: return "GL_FLOAT_MAT3_ARB (0x8B5B)";
    // case 0x8B5C /* GL_FLOAT_MAT4_ARB */: return "GL_FLOAT_MAT4_ARB (0x8B5C)";
    // case 0x8B5D /* GL_SAMPLER_1D_ARB */: return "GL_SAMPLER_1D_ARB (0x8B5D)";
    // case 0x8B5E /* GL_SAMPLER_2D_ARB */: return "GL_SAMPLER_2D_ARB (0x8B5E)";
    // case 0x8B5F /* GL_SAMPLER_3D_ARB */: return "GL_SAMPLER_3D_ARB (0x8B5F)";
    // case 0x8B60 /* GL_SAMPLER_CUBE_ARB */: return "GL_SAMPLER_CUBE_ARB
    // (0x8B60)"; case 0x8B61 /* GL_SAMPLER_1D_SHADOW_ARB */: return
    // "GL_SAMPLER_1D_SHADOW_ARB (0x8B61)"; case 0x8B62 /*
    // GL_SAMPLER_2D_SHADOW_ARB */: return "GL_SAMPLER_2D_SHADOW_ARB (0x8B62)";
    // case 0x8B63 /* GL_SAMPLER_2D_RECT_ARB */: return "GL_SAMPLER_2D_RECT_ARB
    // (0x8B63)"; case 0x8B64 /* GL_SAMPLER_2D_RECT_SHADOW_ARB */: return
    // "GL_SAMPLER_2D_RECT_SHADOW_ARB (0x8B64)"; case 0x8B80 /*
    // GL_OBJECT_DELETE_STATUS_ARB */: return "GL_OBJECT_DELETE_STATUS_ARB
    // (0x8B80)"; case 0x8B81 /* GL_OBJECT_COMPILE_STATUS_ARB */: return
    // "GL_OBJECT_COMPILE_STATUS_ARB (0x8B81)"; case 0x8B82 /*
    // GL_OBJECT_LINK_STATUS_ARB */: return "GL_OBJECT_LINK_STATUS_ARB (0x8B82)";
    // case 0x8B83 /* GL_OBJECT_VALIDATE_STATUS_ARB */: return
    // "GL_OBJECT_VALIDATE_STATUS_ARB (0x8B83)"; case 0x8B84 /*
    // GL_OBJECT_INFO_LOG_LENGTH_ARB */: return "GL_OBJECT_INFO_LOG_LENGTH_ARB
    // (0x8B84)"; case 0x8B85 /* GL_OBJECT_ATTACHED_OBJECTS_ARB */: return
    // "GL_OBJECT_ATTACHED_OBJECTS_ARB (0x8B85)"; case 0x8B86 /*
    // GL_OBJECT_ACTIVE_UNIFORMS_ARB */: return "GL_OBJECT_ACTIVE_UNIFORMS_ARB
    // (0x8B86)"; case 0x8B87 /* GL_OBJECT_ACTIVE_UNIFORM_MAX_LENGTH_ARB */:
    // return "GL_OBJECT_ACTIVE_UNIFORM_MAX_LENGTH_ARB (0x8B87)"; case 0x8B88 /*
    // GL_OBJECT_SHADER_SOURCE_LENGTH_ARB */: return
    // "GL_OBJECT_SHADER_SOURCE_LENGTH_ARB (0x8B88)";

    /* ------------------------ GL_ARB_shader_precision ------------------------
     */

    /* ---------------------- GL_ARB_shader_stencil_export ---------------------
     */

    /* ------------------ GL_ARB_shader_storage_buffer_object ------------------
     */

    // case 0x2000 /* GL_SHADER_STORAGE_BARRIER_BIT */: return
    // "GL_SHADER_STORAGE_BARRIER_BIT (0x2000)"; case 0x8F39 /*
    // GL_MAX_COMBINED_SHADER_OUTPUT_RESOURCES */: return
    // "GL_MAX_COMBINED_SHADER_OUTPUT_RESOURCES (0x8F39)";
    case 0x90D2 /* GL_SHADER_STORAGE_BUFFER */:
      return "GL_SHADER_STORAGE_BUFFER (0x90D2)";
    case 0x90D3 /* GL_SHADER_STORAGE_BUFFER_BINDING */:
      return "GL_SHADER_STORAGE_BUFFER_BINDING (0x90D3)";
    case 0x90D4 /* GL_SHADER_STORAGE_BUFFER_START */:
      return "GL_SHADER_STORAGE_BUFFER_START (0x90D4)";
    case 0x90D5 /* GL_SHADER_STORAGE_BUFFER_SIZE */:
      return "GL_SHADER_STORAGE_BUFFER_SIZE (0x90D5)";
    case 0x90D6 /* GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS */:
      return "GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS (0x90D6)";
    case 0x90D7 /* GL_MAX_GEOMETRY_SHADER_STORAGE_BLOCKS */:
      return "GL_MAX_GEOMETRY_SHADER_STORAGE_BLOCKS (0x90D7)";
    case 0x90D8 /* GL_MAX_TESS_CONTROL_SHADER_STORAGE_BLOCKS */:
      return "GL_MAX_TESS_CONTROL_SHADER_STORAGE_BLOCKS (0x90D8)";
    case 0x90D9 /* GL_MAX_TESS_EVALUATION_SHADER_STORAGE_BLOCKS */:
      return "GL_MAX_TESS_EVALUATION_SHADER_STORAGE_BLOCKS (0x90D9)";
    case 0x90DA /* GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS */:
      return "GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS (0x90DA)";
    case 0x90DB /* GL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS */:
      return "GL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS (0x90DB)";
    case 0x90DC /* GL_MAX_COMBINED_SHADER_STORAGE_BLOCKS */:
      return "GL_MAX_COMBINED_SHADER_STORAGE_BLOCKS (0x90DC)";
    case 0x90DD /* GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS */:
      return "GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS (0x90DD)";
    case 0x90DE /* GL_MAX_SHADER_STORAGE_BLOCK_SIZE */:
      return "GL_MAX_SHADER_STORAGE_BLOCK_SIZE (0x90DE)";
    case 0x90DF /* GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT */:
      return "GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT (0x90DF)"; /* ------------------------
                                                                      GL_ARB_shader_subroutine
                                                                      -----------------------
                                                                    */

    case 0x8DE5 /* GL_ACTIVE_SUBROUTINES */:
      return "GL_ACTIVE_SUBROUTINES (0x8DE5)";
    case 0x8DE6 /* GL_ACTIVE_SUBROUTINE_UNIFORMS */:
      return "GL_ACTIVE_SUBROUTINE_UNIFORMS (0x8DE6)";
    case 0x8DE7 /* GL_MAX_SUBROUTINES */:
      return "GL_MAX_SUBROUTINES (0x8DE7)";
    case 0x8DE8 /* GL_MAX_SUBROUTINE_UNIFORM_LOCATIONS */:
      return "GL_MAX_SUBROUTINE_UNIFORM_LOCATIONS (0x8DE8)";
    case 0x8E47 /* GL_ACTIVE_SUBROUTINE_UNIFORM_LOCATIONS */:
      return "GL_ACTIVE_SUBROUTINE_UNIFORM_LOCATIONS (0x8E47)";
    case 0x8E48 /* GL_ACTIVE_SUBROUTINE_MAX_LENGTH */:
      return "GL_ACTIVE_SUBROUTINE_MAX_LENGTH (0x8E48)";
    case 0x8E49 /* GL_ACTIVE_SUBROUTINE_UNIFORM_MAX_LENGTH */:
      return "GL_ACTIVE_SUBROUTINE_UNIFORM_MAX_LENGTH (0x8E49)";
    case 0x8E4A /* GL_NUM_COMPATIBLE_SUBROUTINES */:
      return "GL_NUM_COMPATIBLE_SUBROUTINES (0x8E4A)";
    case 0x8E4B /* GL_COMPATIBLE_SUBROUTINES */:
      return "GL_COMPATIBLE_SUBROUTINES (0x8E4B)";

      /* ----------------------- GL_ARB_shader_texture_lod
       * ----------------------- */

      /* ---------------------- GL_ARB_shading_language_100
       * ---------------------- */

      // case 0x8B8C /* GL_SHADING_LANGUAGE_VERSION_ARB */: return
      // "GL_SHADING_LANGUAGE_VERSION_ARB (0x8B8C)";

      /* -------------------- GL_ARB_shading_language_420pack
       * -------------------- */

      /* -------------------- GL_ARB_shading_language_include
       * -------------------- */

    case 0x8DAE /* GL_SHADER_INCLUDE_ARB */:
      return "GL_SHADER_INCLUDE_ARB (0x8DAE)";
    case 0x8DE9 /* GL_NAMED_STRING_LENGTH_ARB */:
      return "GL_NAMED_STRING_LENGTH_ARB (0x8DE9)";
    case 0x8DEA /* GL_NAMED_STRING_TYPE_ARB */:
      return "GL_NAMED_STRING_TYPE_ARB (0x8DEA)"; /* --------------------
                                                     GL_ARB_shading_language_packing
                                                     -------------------- */

      /* ----------------------------- GL_ARB_shadow
       * ----------------------------- */

      // case 0x884C /* GL_TEXTURE_COMPARE_MODE_ARB */: return
      // "GL_TEXTURE_COMPARE_MODE_ARB (0x884C)"; case 0x884D /*
      // GL_TEXTURE_COMPARE_FUNC_ARB */: return "GL_TEXTURE_COMPARE_FUNC_ARB
      // (0x884D)"; case 0x884E /* GL_COMPARE_R_TO_TEXTURE_ARB */: return
      // "GL_COMPARE_R_TO_TEXTURE_ARB (0x884E)";

      /* ------------------------- GL_ARB_shadow_ambient
       * ------------------------- */

    case 0x80BF /* GL_TEXTURE_COMPARE_FAIL_VALUE_ARB */:
      return "GL_TEXTURE_COMPARE_FAIL_VALUE_ARB (0x80BF)";

      /* ------------------------ GL_ARB_stencil_texturing
       * ----------------------- */

    case 0x90EA /* GL_DEPTH_STENCIL_TEXTURE_MODE */:
      return "GL_DEPTH_STENCIL_TEXTURE_MODE (0x90EA)";

    /* ------------------------------ GL_ARB_sync ------------------------------
     */

    // case 0x00000001 /* GL_SYNC_FLUSH_COMMANDS_BIT */: return
    // "GL_SYNC_FLUSH_COMMANDS_BIT (0x00000001)";
    case 0x9111 /* GL_MAX_SERVER_WAIT_TIMEOUT */:
      return "GL_MAX_SERVER_WAIT_TIMEOUT (0x9111)";
    case 0x9112 /* GL_OBJECT_TYPE */:
      return "GL_OBJECT_TYPE (0x9112)";
    case 0x9113 /* GL_SYNC_CONDITION */:
      return "GL_SYNC_CONDITION (0x9113)";
    case 0x9114 /* GL_SYNC_STATUS */:
      return "GL_SYNC_STATUS (0x9114)";
    case 0x9115 /* GL_SYNC_FLAGS */:
      return "GL_SYNC_FLAGS (0x9115)";
    case 0x9116 /* GL_SYNC_FENCE */:
      return "GL_SYNC_FENCE (0x9116)";
    case 0x9117 /* GL_SYNC_GPU_COMMANDS_COMPLETE */:
      return "GL_SYNC_GPU_COMMANDS_COMPLETE (0x9117)";
    case 0x9118 /* GL_UNSIGNALED */:
      return "GL_UNSIGNALED (0x9118)";
    case 0x9119 /* GL_SIGNALED */:
      return "GL_SIGNALED (0x9119)";
    case 0x911A /* GL_ALREADY_SIGNALED */:
      return "GL_ALREADY_SIGNALED (0x911A)";
    case 0x911B /* GL_TIMEOUT_EXPIRED */:
      return "GL_TIMEOUT_EXPIRED (0x911B)";
    case 0x911C /* GL_CONDITION_SATISFIED */:
      return "GL_CONDITION_SATISFIED (0x911C)";
    case 0x911D /* GL_WAIT_FAILED */:
      return "GL_WAIT_FAILED (0x911D)";
      // case 0xFFFFFFFFFFFFFFFF /* GL_TIMEOUT_IGNORED */: return
      // "GL_TIMEOUT_IGNORED (0xFFFFFFFFFFFFFFFF)";

      /* ----------------------- GL_ARB_tessellation_shader
       * ---------------------- */

    case 0xE /* GL_PATCHES */:
      return "GL_PATCHES (0xE)";
    case 0x84F0 /* GL_UNIFORM_BLOCK_REFERENCED_BY_TESS_CONTROL_SHADER */:
      return "GL_UNIFORM_BLOCK_REFERENCED_BY_TESS_CONTROL_SHADER (0x84F0)";
    case 0x84F1 /* GL_UNIFORM_BLOCK_REFERENCED_BY_TESS_EVALUATION_SHADER */:
      return "GL_UNIFORM_BLOCK_REFERENCED_BY_TESS_EVALUATION_SHADER (0x84F1)";
    case 0x886C /* GL_MAX_TESS_CONTROL_INPUT_COMPONENTS */:
      return "GL_MAX_TESS_CONTROL_INPUT_COMPONENTS (0x886C)";
    case 0x886D /* GL_MAX_TESS_EVALUATION_INPUT_COMPONENTS */:
      return "GL_MAX_TESS_EVALUATION_INPUT_COMPONENTS (0x886D)";
    case 0x8E1E /* GL_MAX_COMBINED_TESS_CONTROL_UNIFORM_COMPONENTS */:
      return "GL_MAX_COMBINED_TESS_CONTROL_UNIFORM_COMPONENTS (0x8E1E)";
    case 0x8E1F /* GL_MAX_COMBINED_TESS_EVALUATION_UNIFORM_COMPONENTS */:
      return "GL_MAX_COMBINED_TESS_EVALUATION_UNIFORM_COMPONENTS (0x8E1F)";
    case 0x8E72 /* GL_PATCH_VERTICES */:
      return "GL_PATCH_VERTICES (0x8E72)";
    case 0x8E73 /* GL_PATCH_DEFAULT_INNER_LEVEL */:
      return "GL_PATCH_DEFAULT_INNER_LEVEL (0x8E73)";
    case 0x8E74 /* GL_PATCH_DEFAULT_OUTER_LEVEL */:
      return "GL_PATCH_DEFAULT_OUTER_LEVEL (0x8E74)";
    case 0x8E75 /* GL_TESS_CONTROL_OUTPUT_VERTICES */:
      return "GL_TESS_CONTROL_OUTPUT_VERTICES (0x8E75)";
    case 0x8E76 /* GL_TESS_GEN_MODE */:
      return "GL_TESS_GEN_MODE (0x8E76)";
    case 0x8E77 /* GL_TESS_GEN_SPACING */:
      return "GL_TESS_GEN_SPACING (0x8E77)";
    case 0x8E78 /* GL_TESS_GEN_VERTEX_ORDER */:
      return "GL_TESS_GEN_VERTEX_ORDER (0x8E78)";
    case 0x8E79 /* GL_TESS_GEN_POINT_MODE */:
      return "GL_TESS_GEN_POINT_MODE (0x8E79)";
    case 0x8E7A /* GL_ISOLINES */:
      return "GL_ISOLINES (0x8E7A)";
    case 0x8E7B /* GL_FRACTIONAL_ODD */:
      return "GL_FRACTIONAL_ODD (0x8E7B)";
    case 0x8E7C /* GL_FRACTIONAL_EVEN */:
      return "GL_FRACTIONAL_EVEN (0x8E7C)";
    case 0x8E7D /* GL_MAX_PATCH_VERTICES */:
      return "GL_MAX_PATCH_VERTICES (0x8E7D)";
    case 0x8E7E /* GL_MAX_TESS_GEN_LEVEL */:
      return "GL_MAX_TESS_GEN_LEVEL (0x8E7E)";
    case 0x8E7F /* GL_MAX_TESS_CONTROL_UNIFORM_COMPONENTS */:
      return "GL_MAX_TESS_CONTROL_UNIFORM_COMPONENTS (0x8E7F)";
    case 0x8E80 /* GL_MAX_TESS_EVALUATION_UNIFORM_COMPONENTS */:
      return "GL_MAX_TESS_EVALUATION_UNIFORM_COMPONENTS (0x8E80)";
    case 0x8E81 /* GL_MAX_TESS_CONTROL_TEXTURE_IMAGE_UNITS */:
      return "GL_MAX_TESS_CONTROL_TEXTURE_IMAGE_UNITS (0x8E81)";
    case 0x8E82 /* GL_MAX_TESS_EVALUATION_TEXTURE_IMAGE_UNITS */:
      return "GL_MAX_TESS_EVALUATION_TEXTURE_IMAGE_UNITS (0x8E82)";
    case 0x8E83 /* GL_MAX_TESS_CONTROL_OUTPUT_COMPONENTS */:
      return "GL_MAX_TESS_CONTROL_OUTPUT_COMPONENTS (0x8E83)";
    case 0x8E84 /* GL_MAX_TESS_PATCH_COMPONENTS */:
      return "GL_MAX_TESS_PATCH_COMPONENTS (0x8E84)";
    case 0x8E85 /* GL_MAX_TESS_CONTROL_TOTAL_OUTPUT_COMPONENTS */:
      return "GL_MAX_TESS_CONTROL_TOTAL_OUTPUT_COMPONENTS (0x8E85)";
    case 0x8E86 /* GL_MAX_TESS_EVALUATION_OUTPUT_COMPONENTS */:
      return "GL_MAX_TESS_EVALUATION_OUTPUT_COMPONENTS (0x8E86)";
    case 0x8E87 /* GL_TESS_EVALUATION_SHADER */:
      return "GL_TESS_EVALUATION_SHADER (0x8E87)";
    case 0x8E88 /* GL_TESS_CONTROL_SHADER */:
      return "GL_TESS_CONTROL_SHADER (0x8E88)";
    case 0x8E89 /* GL_MAX_TESS_CONTROL_UNIFORM_BLOCKS */:
      return "GL_MAX_TESS_CONTROL_UNIFORM_BLOCKS (0x8E89)";
    case 0x8E8A /* GL_MAX_TESS_EVALUATION_UNIFORM_BLOCKS */:
      return "GL_MAX_TESS_EVALUATION_UNIFORM_BLOCKS (0x8E8A)";

      /* ---------------------- GL_ARB_texture_border_clamp
       * ---------------------- */

      // case 0x812D /* GL_CLAMP_TO_BORDER_ARB */: return
      // "GL_CLAMP_TO_BORDER_ARB (0x812D)";

      /* ---------------------- GL_ARB_texture_buffer_object
       * --------------------- */

      // case 0x8C2A /* GL_TEXTURE_BUFFER_ARB */: return "GL_TEXTURE_BUFFER_ARB
      // (0x8C2A)"; case 0x8C2B /* GL_MAX_TEXTURE_BUFFER_SIZE_ARB */: return
      // "GL_MAX_TEXTURE_BUFFER_SIZE_ARB (0x8C2B)"; case 0x8C2C /*
      // GL_TEXTURE_BINDING_BUFFER_ARB */: return "GL_TEXTURE_BINDING_BUFFER_ARB
      // (0x8C2C)"; case 0x8C2D /* GL_TEXTURE_BUFFER_DATA_STORE_BINDING_ARB */:
      // return "GL_TEXTURE_BUFFER_DATA_STORE_BINDING_ARB (0x8C2D)"; case 0x8C2E
      // /* GL_TEXTURE_BUFFER_FORMAT_ARB */: return "GL_TEXTURE_BUFFER_FORMAT_ARB
      // (0x8C2E)";/* ------------------- GL_ARB_texture_buffer_object_rgb32
      // ------------------ */

      /* ---------------------- GL_ARB_texture_buffer_range
       * ---------------------- */

    case 0x919D /* GL_TEXTURE_BUFFER_OFFSET */:
      return "GL_TEXTURE_BUFFER_OFFSET (0x919D)";
    case 0x919E /* GL_TEXTURE_BUFFER_SIZE */:
      return "GL_TEXTURE_BUFFER_SIZE (0x919E)";
    case 0x919F /* GL_TEXTURE_BUFFER_OFFSET_ALIGNMENT */:
      return "GL_TEXTURE_BUFFER_OFFSET_ALIGNMENT (0x919F)";

      /* ----------------------- GL_ARB_texture_compression
       * ---------------------- */

      // case 0x84E9 /* GL_COMPRESSED_ALPHA_ARB */: return
      // "GL_COMPRESSED_ALPHA_ARB (0x84E9)"; case 0x84EA /*
      // GL_COMPRESSED_LUMINANCE_ARB */: return "GL_COMPRESSED_LUMINANCE_ARB
      // (0x84EA)"; case 0x84EB /* GL_COMPRESSED_LUMINANCE_ALPHA_ARB */: return
      // "GL_COMPRESSED_LUMINANCE_ALPHA_ARB (0x84EB)"; case 0x84EC /*
      // GL_COMPRESSED_INTENSITY_ARB */: return "GL_COMPRESSED_INTENSITY_ARB
      // (0x84EC)"; case 0x84ED /* GL_COMPRESSED_RGB_ARB */: return
      // "GL_COMPRESSED_RGB_ARB (0x84ED)"; case 0x84EE /* GL_COMPRESSED_RGBA_ARB
      // */: return "GL_COMPRESSED_RGBA_ARB (0x84EE)"; case 0x84EF /*
      // GL_TEXTURE_COMPRESSION_HINT_ARB */: return
      // "GL_TEXTURE_COMPRESSION_HINT_ARB (0x84EF)"; case 0x86A0 /*
      // GL_TEXTURE_COMPRESSED_IMAGE_SIZE_ARB */: return
      // "GL_TEXTURE_COMPRESSED_IMAGE_SIZE_ARB (0x86A0)"; case 0x86A1 /*
      // GL_TEXTURE_COMPRESSED_ARB */: return "GL_TEXTURE_COMPRESSED_ARB
      // (0x86A1)"; case 0x86A2 /* GL_NUM_COMPRESSED_TEXTURE_FORMATS_ARB */:
      // return "GL_NUM_COMPRESSED_TEXTURE_FORMATS_ARB (0x86A2)"; case 0x86A3 /*
      // GL_COMPRESSED_TEXTURE_FORMATS_ARB */: return
      // "GL_COMPRESSED_TEXTURE_FORMATS_ARB (0x86A3)";

      /* -------------------- GL_ARB_texture_compression_bptc
       * -------------------- */

      // case 0x8E8C /* GL_COMPRESSED_RGBA_BPTC_UNORM_ARB */: return
      // "GL_COMPRESSED_RGBA_BPTC_UNORM_ARB (0x8E8C)"; case 0x8E8D /*
      // GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_ARB */: return
      // "GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_ARB (0x8E8D)"; case 0x8E8E /*
      // GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_ARB */: return
      // "GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_ARB (0x8E8E)"; case 0x8E8F /*
      // GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_ARB */: return
      // "GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_ARB (0x8E8F)";

      /* -------------------- GL_ARB_texture_compression_rgtc
       * -------------------- */

    case 0x8DBB /* GL_COMPRESSED_RED_RGTC1 */:
      return "GL_COMPRESSED_RED_RGTC1 (0x8DBB)";
    case 0x8DBC /* GL_COMPRESSED_SIGNED_RED_RGTC1 */:
      return "GL_COMPRESSED_SIGNED_RED_RGTC1 (0x8DBC)";
    case 0x8DBD /* GL_COMPRESSED_RG_RGTC2 */:
      return "GL_COMPRESSED_RG_RGTC2 (0x8DBD)";
    case 0x8DBE /* GL_COMPRESSED_SIGNED_RG_RGTC2 */:
      return "GL_COMPRESSED_SIGNED_RG_RGTC2 (0x8DBE)";

      /* ------------------------ GL_ARB_texture_cube_map
       * ------------------------ */

      // case 0x8511 /* GL_NORMAL_MAP_ARB */: return "GL_NORMAL_MAP_ARB
      // (0x8511)"; case 0x8512 /* GL_REFLECTION_MAP_ARB */: return
      // "GL_REFLECTION_MAP_ARB (0x8512)"; case 0x8513 /* GL_TEXTURE_CUBE_MAP_ARB
      // */: return "GL_TEXTURE_CUBE_MAP_ARB (0x8513)"; case 0x8514 /*
      // GL_TEXTURE_BINDING_CUBE_MAP_ARB */: return
      // "GL_TEXTURE_BINDING_CUBE_MAP_ARB (0x8514)"; case 0x8515 /*
      // GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB */: return
      // "GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB (0x8515)"; case 0x8516 /*
      // GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB */: return
      // "GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB (0x8516)"; case 0x8517 /*
      // GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB */: return
      // "GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB (0x8517)"; case 0x8518 /*
      // GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB */: return
      // "GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB (0x8518)"; case 0x8519 /*
      // GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB */: return
      // "GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB (0x8519)"; case 0x851A /*
      // GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB */: return
      // "GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB (0x851A)"; case 0x851B /*
      // GL_PROXY_TEXTURE_CUBE_MAP_ARB */: return "GL_PROXY_TEXTURE_CUBE_MAP_ARB
      // (0x851B)"; case 0x851C /* GL_MAX_CUBE_MAP_TEXTURE_SIZE_ARB */: return
      // "GL_MAX_CUBE_MAP_TEXTURE_SIZE_ARB (0x851C)";

      /* --------------------- GL_ARB_texture_cube_map_array
       * --------------------- */

      // case 0x9009 /* GL_TEXTURE_CUBE_MAP_ARRAY_ARB */: return
      // "GL_TEXTURE_CUBE_MAP_ARRAY_ARB (0x9009)"; case 0x900A /*
      // GL_TEXTURE_BINDING_CUBE_MAP_ARRAY_ARB */: return
      // "GL_TEXTURE_BINDING_CUBE_MAP_ARRAY_ARB (0x900A)"; case 0x900B /*
      // GL_PROXY_TEXTURE_CUBE_MAP_ARRAY_ARB */: return
      // "GL_PROXY_TEXTURE_CUBE_MAP_ARRAY_ARB (0x900B)"; case 0x900C /*
      // GL_SAMPLER_CUBE_MAP_ARRAY_ARB */: return "GL_SAMPLER_CUBE_MAP_ARRAY_ARB
      // (0x900C)"; case 0x900D /* GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW_ARB */:
      // return "GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW_ARB (0x900D)"; case 0x900E /*
      // GL_INT_SAMPLER_CUBE_MAP_ARRAY_ARB */: return
      // "GL_INT_SAMPLER_CUBE_MAP_ARRAY_ARB (0x900E)"; case 0x900F /*
      // GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY_ARB */: return
      // "GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY_ARB (0x900F)";

      /* ------------------------- GL_ARB_texture_env_add
       * ------------------------ */

      /* ----------------------- GL_ARB_texture_env_combine
       * ---------------------- */

      // case 0x84E7 /* GL_SUBTRACT_ARB */: return "GL_SUBTRACT_ARB (0x84E7)";
      // case 0x8570 /* GL_COMBINE_ARB */: return "GL_COMBINE_ARB (0x8570)";
      // case 0x8571 /* GL_COMBINE_RGB_ARB */: return "GL_COMBINE_RGB_ARB
      // (0x8571)"; case 0x8572 /* GL_COMBINE_ALPHA_ARB */: return
      // "GL_COMBINE_ALPHA_ARB (0x8572)"; case 0x8573 /* GL_RGB_SCALE_ARB */:
      // return "GL_RGB_SCALE_ARB (0x8573)"; case 0x8574 /* GL_ADD_SIGNED_ARB */:
      // return "GL_ADD_SIGNED_ARB (0x8574)"; case 0x8575 /* GL_INTERPOLATE_ARB
      // */: return "GL_INTERPOLATE_ARB (0x8575)"; case 0x8576 /* GL_CONSTANT_ARB
      // */: return "GL_CONSTANT_ARB (0x8576)"; case 0x8577 /*
      // GL_PRIMARY_COLOR_ARB */: return "GL_PRIMARY_COLOR_ARB (0x8577)"; case
      // 0x8578 /* GL_PREVIOUS_ARB */: return "GL_PREVIOUS_ARB (0x8578)"; case
      // 0x8580 /* GL_SOURCE0_RGB_ARB */: return "GL_SOURCE0_RGB_ARB (0x8580)";
      // case 0x8581 /* GL_SOURCE1_RGB_ARB */: return "GL_SOURCE1_RGB_ARB
      // (0x8581)"; case 0x8582 /* GL_SOURCE2_RGB_ARB */: return
      // "GL_SOURCE2_RGB_ARB (0x8582)"; case 0x8588 /* GL_SOURCE0_ALPHA_ARB */:
      // return "GL_SOURCE0_ALPHA_ARB (0x8588)"; case 0x8589 /*
      // GL_SOURCE1_ALPHA_ARB */: return "GL_SOURCE1_ALPHA_ARB (0x8589)"; case
      // 0x858A /* GL_SOURCE2_ALPHA_ARB */: return "GL_SOURCE2_ALPHA_ARB
      // (0x858A)"; case 0x8590 /* GL_OPERAND0_RGB_ARB */: return
      // "GL_OPERAND0_RGB_ARB (0x8590)"; case 0x8591 /* GL_OPERAND1_RGB_ARB */:
      // return "GL_OPERAND1_RGB_ARB (0x8591)"; case 0x8592 /*
      // GL_OPERAND2_RGB_ARB */: return "GL_OPERAND2_RGB_ARB (0x8592)"; case
      // 0x8598 /* GL_OPERAND0_ALPHA_ARB */: return "GL_OPERAND0_ALPHA_ARB
      // (0x8598)"; case 0x8599 /* GL_OPERAND1_ALPHA_ARB */: return
      // "GL_OPERAND1_ALPHA_ARB (0x8599)"; case 0x859A /* GL_OPERAND2_ALPHA_ARB
      // */: return "GL_OPERAND2_ALPHA_ARB (0x859A)";

      /* ---------------------- GL_ARB_texture_env_crossbar
       * ---------------------- */

      /* ------------------------ GL_ARB_texture_env_dot3
       * ------------------------ */

      // case 0x86AE /* GL_DOT3_RGB_ARB */: return "GL_DOT3_RGB_ARB (0x86AE)";
      // case 0x86AF /* GL_DOT3_RGBA_ARB */: return "GL_DOT3_RGBA_ARB (0x86AF)";

      /* -------------------------- GL_ARB_texture_float
       * ------------------------- */

      // case 0x8815 /* GL_RGB32F_ARB */: return "GL_RGB32F_ARB (0x8815)";
      // case 0x8816 /* GL_ALPHA32F_ARB */: return "GL_ALPHA32F_ARB (0x8816)";
      // case 0x8817 /* GL_INTENSITY32F_ARB */: return "GL_INTENSITY32F_ARB
      // (0x8817)"; case 0x8818 /* GL_LUMINANCE32F_ARB */: return
      // "GL_LUMINANCE32F_ARB (0x8818)"; case 0x8819 /* GL_LUMINANCE_ALPHA32F_ARB
      // */: return "GL_LUMINANCE_ALPHA32F_ARB (0x8819)"; case 0x881A /*
      // GL_RGBA16F_ARB */: return "GL_RGBA16F_ARB (0x881A)"; case 0x881B /*
      // GL_RGB16F_ARB */: return "GL_RGB16F_ARB (0x881B)"; case 0x881C /*
      // GL_ALPHA16F_ARB */: return "GL_ALPHA16F_ARB (0x881C)"; case 0x881D /*
      // GL_INTENSITY16F_ARB */: return "GL_INTENSITY16F_ARB (0x881D)"; case
      // 0x881E /* GL_LUMINANCE16F_ARB */: return "GL_LUMINANCE16F_ARB (0x881E)";
      // case 0x881F /* GL_LUMINANCE_ALPHA16F_ARB */: return
      // "GL_LUMINANCE_ALPHA16F_ARB (0x881F)"; case 0x8C10 /*
      // GL_TEXTURE_RED_TYPE_ARB */: return "GL_TEXTURE_RED_TYPE_ARB (0x8C10)";
      // case 0x8C11 /* GL_TEXTURE_GREEN_TYPE_ARB */: return
      // "GL_TEXTURE_GREEN_TYPE_ARB (0x8C11)"; case 0x8C12 /*
      // GL_TEXTURE_BLUE_TYPE_ARB */: return "GL_TEXTURE_BLUE_TYPE_ARB (0x8C12)";
      // case 0x8C13 /* GL_TEXTURE_ALPHA_TYPE_ARB */: return
      // "GL_TEXTURE_ALPHA_TYPE_ARB (0x8C13)"; case 0x8C14 /*
      // GL_TEXTURE_LUMINANCE_TYPE_ARB */: return "GL_TEXTURE_LUMINANCE_TYPE_ARB
      // (0x8C14)"; case 0x8C15 /* GL_TEXTURE_INTENSITY_TYPE_ARB */: return
      // "GL_TEXTURE_INTENSITY_TYPE_ARB (0x8C15)"; case 0x8C16 /*
      // GL_TEXTURE_DEPTH_TYPE_ARB */: return "GL_TEXTURE_DEPTH_TYPE_ARB
      // (0x8C16)"; case 0x8C17 /* GL_UNSIGNED_NORMALIZED_ARB */: return
      // "GL_UNSIGNED_NORMALIZED_ARB (0x8C17)";

      /* ------------------------- GL_ARB_texture_gather
       * ------------------------- */

      // case 0x8E5E /* GL_MIN_PROGRAM_TEXTURE_GATHER_OFFSET_ARB */: return
      // "GL_MIN_PROGRAM_TEXTURE_GATHER_OFFSET_ARB (0x8E5E)"; case 0x8E5F /*
      // GL_MAX_PROGRAM_TEXTURE_GATHER_OFFSET_ARB */: return
      // "GL_MAX_PROGRAM_TEXTURE_GATHER_OFFSET_ARB (0x8E5F)"; case 0x8F9F /*
      // GL_MAX_PROGRAM_TEXTURE_GATHER_COMPONENTS_ARB */: return
      // "GL_MAX_PROGRAM_TEXTURE_GATHER_COMPONENTS_ARB (0x8F9F)";

      /* --------------------- GL_ARB_texture_mirrored_repeat
       * -------------------- */

      // case 0x8370 /* GL_MIRRORED_REPEAT_ARB */: return
      // "GL_MIRRORED_REPEAT_ARB (0x8370)";

      /* ----------------------- GL_ARB_texture_multisample
       * ---------------------- */

    case 0x8E50 /* GL_SAMPLE_POSITION */:
      return "GL_SAMPLE_POSITION (0x8E50)";
    case 0x8E51 /* GL_SAMPLE_MASK */:
      return "GL_SAMPLE_MASK (0x8E51)";
    case 0x8E52 /* GL_SAMPLE_MASK_VALUE */:
      return "GL_SAMPLE_MASK_VALUE (0x8E52)";
    case 0x8E59 /* GL_MAX_SAMPLE_MASK_WORDS */:
      return "GL_MAX_SAMPLE_MASK_WORDS (0x8E59)";
    // case 0x9100 /* GL_TEXTURE_2D_MULTISAMPLE */: return
    // "GL_TEXTURE_2D_MULTISAMPLE (0x9100)";
    case 0x9101 /* GL_PROXY_TEXTURE_2D_MULTISAMPLE */:
      return "GL_PROXY_TEXTURE_2D_MULTISAMPLE (0x9101)";
    // case 0x9102 /* GL_TEXTURE_2D_MULTISAMPLE_ARRAY */: return
    // "GL_TEXTURE_2D_MULTISAMPLE_ARRAY (0x9102)";
    case 0x9103 /* GL_PROXY_TEXTURE_2D_MULTISAMPLE_ARRAY */:
      return "GL_PROXY_TEXTURE_2D_MULTISAMPLE_ARRAY (0x9103)";
    case 0x9104 /* GL_TEXTURE_BINDING_2D_MULTISAMPLE */:
      return "GL_TEXTURE_BINDING_2D_MULTISAMPLE (0x9104)";
    case 0x9105 /* GL_TEXTURE_BINDING_2D_MULTISAMPLE_ARRAY */:
      return "GL_TEXTURE_BINDING_2D_MULTISAMPLE_ARRAY (0x9105)";
    case 0x9106 /* GL_TEXTURE_SAMPLES */:
      return "GL_TEXTURE_SAMPLES (0x9106)";
    case 0x9107 /* GL_TEXTURE_FIXED_SAMPLE_LOCATIONS */:
      return "GL_TEXTURE_FIXED_SAMPLE_LOCATIONS (0x9107)";
    case 0x9108 /* GL_SAMPLER_2D_MULTISAMPLE */:
      return "GL_SAMPLER_2D_MULTISAMPLE (0x9108)";
    case 0x9109 /* GL_INT_SAMPLER_2D_MULTISAMPLE */:
      return "GL_INT_SAMPLER_2D_MULTISAMPLE (0x9109)";
    case 0x910A /* GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE */:
      return "GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE (0x910A)";
    case 0x910B /* GL_SAMPLER_2D_MULTISAMPLE_ARRAY */:
      return "GL_SAMPLER_2D_MULTISAMPLE_ARRAY (0x910B)";
    case 0x910C /* GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY */:
      return "GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY (0x910C)";
    case 0x910D /* GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY */:
      return "GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY (0x910D)";
    case 0x910E /* GL_MAX_COLOR_TEXTURE_SAMPLES */:
      return "GL_MAX_COLOR_TEXTURE_SAMPLES (0x910E)";
    case 0x910F /* GL_MAX_DEPTH_TEXTURE_SAMPLES */:
      return "GL_MAX_DEPTH_TEXTURE_SAMPLES (0x910F)";
    case 0x9110 /* GL_MAX_INTEGER_SAMPLES */:
      return "GL_MAX_INTEGER_SAMPLES (0x9110)";

    /* -------------------- GL_ARB_texture_non_power_of_two --------------------
     */

    /* ---------------------- GL_ARB_texture_query_levels ----------------------
     */

    /* ------------------------ GL_ARB_texture_query_lod -----------------------
     */

    /* ------------------------ GL_ARB_texture_rectangle -----------------------
     */

    // case 0x84F5 /* GL_TEXTURE_RECTANGLE_ARB */: return
    // "GL_TEXTURE_RECTANGLE_ARB (0x84F5)"; case 0x84F6 /*
    // GL_TEXTURE_BINDING_RECTANGLE_ARB */: return
    // "GL_TEXTURE_BINDING_RECTANGLE_ARB (0x84F6)"; case 0x84F7 /*
    // GL_PROXY_TEXTURE_RECTANGLE_ARB */: return "GL_PROXY_TEXTURE_RECTANGLE_ARB
    // (0x84F7)"; case 0x84F8 /* GL_MAX_RECTANGLE_TEXTURE_SIZE_ARB */: return
    // "GL_MAX_RECTANGLE_TEXTURE_SIZE_ARB (0x84F8)"; case 0x8B63 /*
    // GL_SAMPLER_2D_RECT_ARB */: return "GL_SAMPLER_2D_RECT_ARB (0x8B63)"; case
    // 0x8B64 /* GL_SAMPLER_2D_RECT_SHADOW_ARB */: return
    // "GL_SAMPLER_2D_RECT_SHADOW_ARB (0x8B64)";

    /* --------------------------- GL_ARB_texture_rg ---------------------------
     */

    // case 0x8225 /* GL_COMPRESSED_RED */: return "GL_COMPRESSED_RED (0x8225)";
    // case 0x8226 /* GL_COMPRESSED_RG */: return "GL_COMPRESSED_RG (0x8226)";
    case 0x8227 /* GL_RG */:
      return "GL_RG (0x8227)";
    case 0x8228 /* GL_RG_INTEGER */:
      return "GL_RG_INTEGER (0x8228)";
    case 0x8229 /* GL_R8 */:
      return "GL_R8 (0x8229)";
    case 0x822A /* GL_R16 */:
      return "GL_R16 (0x822A)";
    case 0x822B /* GL_RG8 */:
      return "GL_RG8 (0x822B)";
    case 0x822C /* GL_RG16 */:
      return "GL_RG16 (0x822C)";
    case 0x822D /* GL_R16F */:
      return "GL_R16F (0x822D)";
    case 0x822E /* GL_R32F */:
      return "GL_R32F (0x822E)";
    case 0x822F /* GL_RG16F */:
      return "GL_RG16F (0x822F)";
    case 0x8230 /* GL_RG32F */:
      return "GL_RG32F (0x8230)";
    case 0x8231 /* GL_R8I */:
      return "GL_R8I (0x8231)";
    case 0x8232 /* GL_R8UI */:
      return "GL_R8UI (0x8232)";
    case 0x8233 /* GL_R16I */:
      return "GL_R16I (0x8233)";
    case 0x8234 /* GL_R16UI */:
      return "GL_R16UI (0x8234)";
    case 0x8235 /* GL_R32I */:
      return "GL_R32I (0x8235)";
    case 0x8236 /* GL_R32UI */:
      return "GL_R32UI (0x8236)";
    case 0x8237 /* GL_RG8I */:
      return "GL_RG8I (0x8237)";
    case 0x8238 /* GL_RG8UI */:
      return "GL_RG8UI (0x8238)";
    case 0x8239 /* GL_RG16I */:
      return "GL_RG16I (0x8239)";
    case 0x823A /* GL_RG16UI */:
      return "GL_RG16UI (0x823A)";
    case 0x823B /* GL_RG32I */:
      return "GL_RG32I (0x823B)";
    case 0x823C /* GL_RG32UI */:
      return "GL_RG32UI (0x823C)";

      /* ----------------------- GL_ARB_texture_rgb10_a2ui
       * ----------------------- */

      // case 0x906F /* GL_RGB10_A2UI */: return "GL_RGB10_A2UI (0x906F)";

      /* ------------------------- GL_ARB_texture_storage
       * ------------------------ */

    case 0x912F /* GL_TEXTURE_IMMUTABLE_FORMAT */:
      return "GL_TEXTURE_IMMUTABLE_FORMAT (0x912F)"; /* -------------------
                                                        GL_ARB_texture_storage_multisample
                                                        ------------------ */

      /* ------------------------- GL_ARB_texture_swizzle
       * ------------------------ */

      // case 0x8E42 /* GL_TEXTURE_SWIZZLE_R */: return "GL_TEXTURE_SWIZZLE_R
      // (0x8E42)"; case 0x8E43 /* GL_TEXTURE_SWIZZLE_G */: return
      // "GL_TEXTURE_SWIZZLE_G (0x8E43)"; case 0x8E44 /* GL_TEXTURE_SWIZZLE_B */:
      // return "GL_TEXTURE_SWIZZLE_B (0x8E44)"; case 0x8E45 /*
      // GL_TEXTURE_SWIZZLE_A */: return "GL_TEXTURE_SWIZZLE_A (0x8E45)"; case
      // 0x8E46 /* GL_TEXTURE_SWIZZLE_RGBA */: return "GL_TEXTURE_SWIZZLE_RGBA
      // (0x8E46)";

      /* -------------------------- GL_ARB_texture_view
       * -------------------------- */

    case 0x82DB /* GL_TEXTURE_VIEW_MIN_LEVEL */:
      return "GL_TEXTURE_VIEW_MIN_LEVEL (0x82DB)";
    case 0x82DC /* GL_TEXTURE_VIEW_NUM_LEVELS */:
      return "GL_TEXTURE_VIEW_NUM_LEVELS (0x82DC)";
    case 0x82DD /* GL_TEXTURE_VIEW_MIN_LAYER */:
      return "GL_TEXTURE_VIEW_MIN_LAYER (0x82DD)";
    case 0x82DE /* GL_TEXTURE_VIEW_NUM_LAYERS */:
      return "GL_TEXTURE_VIEW_NUM_LAYERS (0x82DE)";
    case 0x82DF /* GL_TEXTURE_IMMUTABLE_LEVELS */:
      return "GL_TEXTURE_IMMUTABLE_LEVELS (0x82DF)"; /* ---------------------------
                                                        GL_ARB_timer_query
                                                        --------------------------
                                                      */

    case 0x88BF /* GL_TIME_ELAPSED */:
      return "GL_TIME_ELAPSED (0x88BF)";
    case 0x8E28 /* GL_TIMESTAMP */:
      return "GL_TIMESTAMP (0x8E28)";

      /* ----------------------- GL_ARB_transform_feedback2
       * ---------------------- */

    case 0x8E22 /* GL_TRANSFORM_FEEDBACK */:
      return "GL_TRANSFORM_FEEDBACK (0x8E22)";
    case 0x8E23 /* GL_TRANSFORM_FEEDBACK_BUFFER_PAUSED */:
      return "GL_TRANSFORM_FEEDBACK_BUFFER_PAUSED (0x8E23)";
    case 0x8E24 /* GL_TRANSFORM_FEEDBACK_BUFFER_ACTIVE */:
      return "GL_TRANSFORM_FEEDBACK_BUFFER_ACTIVE (0x8E24)";
    case 0x8E25 /* GL_TRANSFORM_FEEDBACK_BINDING */:
      return "GL_TRANSFORM_FEEDBACK_BINDING (0x8E25)";

      /* ----------------------- GL_ARB_transform_feedback3
       * ---------------------- */

    case 0x8E70 /* GL_MAX_TRANSFORM_FEEDBACK_BUFFERS */:
      return "GL_MAX_TRANSFORM_FEEDBACK_BUFFERS (0x8E70)";
      // case 0x8E71 /* GL_MAX_VERTEX_STREAMS */: return "GL_MAX_VERTEX_STREAMS
      // (0x8E71)";

      /* ------------------ GL_ARB_transform_feedback_instanced
       * ------------------ */

      /* ------------------------ GL_ARB_transpose_matrix
       * ------------------------ */

      // case 0x84E3 /* GL_TRANSPOSE_MODELVIEW_MATRIX_ARB */: return
      // "GL_TRANSPOSE_MODELVIEW_MATRIX_ARB (0x84E3)"; case 0x84E4 /*
      // GL_TRANSPOSE_PROJECTION_MATRIX_ARB */: return
      // "GL_TRANSPOSE_PROJECTION_MATRIX_ARB (0x84E4)"; case 0x84E5 /*
      // GL_TRANSPOSE_TEXTURE_MATRIX_ARB */: return
      // "GL_TRANSPOSE_TEXTURE_MATRIX_ARB (0x84E5)"; case 0x84E6 /*
      // GL_TRANSPOSE_COLOR_MATRIX_ARB */: return "GL_TRANSPOSE_COLOR_MATRIX_ARB
      // (0x84E6)";

      /* ---------------------- GL_ARB_uniform_buffer_object
       * --------------------- */

    case 0x8A11 /* GL_UNIFORM_BUFFER */:
      return "GL_UNIFORM_BUFFER (0x8A11)";
    case 0x8A28 /* GL_UNIFORM_BUFFER_BINDING */:
      return "GL_UNIFORM_BUFFER_BINDING (0x8A28)";
    case 0x8A29 /* GL_UNIFORM_BUFFER_START */:
      return "GL_UNIFORM_BUFFER_START (0x8A29)";
    case 0x8A2A /* GL_UNIFORM_BUFFER_SIZE */:
      return "GL_UNIFORM_BUFFER_SIZE (0x8A2A)";
    case 0x8A2B /* GL_MAX_VERTEX_UNIFORM_BLOCKS */:
      return "GL_MAX_VERTEX_UNIFORM_BLOCKS (0x8A2B)";
    case 0x8A2C /* GL_MAX_GEOMETRY_UNIFORM_BLOCKS */:
      return "GL_MAX_GEOMETRY_UNIFORM_BLOCKS (0x8A2C)";
    case 0x8A2D /* GL_MAX_FRAGMENT_UNIFORM_BLOCKS */:
      return "GL_MAX_FRAGMENT_UNIFORM_BLOCKS (0x8A2D)";
    case 0x8A2E /* GL_MAX_COMBINED_UNIFORM_BLOCKS */:
      return "GL_MAX_COMBINED_UNIFORM_BLOCKS (0x8A2E)";
    case 0x8A2F /* GL_MAX_UNIFORM_BUFFER_BINDINGS */:
      return "GL_MAX_UNIFORM_BUFFER_BINDINGS (0x8A2F)";
    case 0x8A30 /* GL_MAX_UNIFORM_BLOCK_SIZE */:
      return "GL_MAX_UNIFORM_BLOCK_SIZE (0x8A30)";
    case 0x8A31 /* GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS */:
      return "GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS (0x8A31)";
    case 0x8A32 /* GL_MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENTS */:
      return "GL_MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENTS (0x8A32)";
    case 0x8A33 /* GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS */:
      return "GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS (0x8A33)";
    case 0x8A34 /* GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT */:
      return "GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT (0x8A34)";
    case 0x8A35 /* GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH */:
      return "GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH (0x8A35)";
    case 0x8A36 /* GL_ACTIVE_UNIFORM_BLOCKS */:
      return "GL_ACTIVE_UNIFORM_BLOCKS (0x8A36)";
    case 0x8A37 /* GL_UNIFORM_TYPE */:
      return "GL_UNIFORM_TYPE (0x8A37)";
    case 0x8A38 /* GL_UNIFORM_SIZE */:
      return "GL_UNIFORM_SIZE (0x8A38)";
    case 0x8A39 /* GL_UNIFORM_NAME_LENGTH */:
      return "GL_UNIFORM_NAME_LENGTH (0x8A39)";
    case 0x8A3A /* GL_UNIFORM_BLOCK_INDEX */:
      return "GL_UNIFORM_BLOCK_INDEX (0x8A3A)";
    case 0x8A3B /* GL_UNIFORM_OFFSET */:
      return "GL_UNIFORM_OFFSET (0x8A3B)";
    case 0x8A3C /* GL_UNIFORM_ARRAY_STRIDE */:
      return "GL_UNIFORM_ARRAY_STRIDE (0x8A3C)";
    case 0x8A3D /* GL_UNIFORM_MATRIX_STRIDE */:
      return "GL_UNIFORM_MATRIX_STRIDE (0x8A3D)";
    case 0x8A3E /* GL_UNIFORM_IS_ROW_MAJOR */:
      return "GL_UNIFORM_IS_ROW_MAJOR (0x8A3E)";
    case 0x8A3F /* GL_UNIFORM_BLOCK_BINDING */:
      return "GL_UNIFORM_BLOCK_BINDING (0x8A3F)";
    case 0x8A40 /* GL_UNIFORM_BLOCK_DATA_SIZE */:
      return "GL_UNIFORM_BLOCK_DATA_SIZE (0x8A40)";
    case 0x8A41 /* GL_UNIFORM_BLOCK_NAME_LENGTH */:
      return "GL_UNIFORM_BLOCK_NAME_LENGTH (0x8A41)";
    case 0x8A42 /* GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS */:
      return "GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS (0x8A42)";
    case 0x8A43 /* GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES */:
      return "GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES (0x8A43)";
    case 0x8A44 /* GL_UNIFORM_BLOCK_REFERENCED_BY_VERTEX_SHADER */:
      return "GL_UNIFORM_BLOCK_REFERENCED_BY_VERTEX_SHADER (0x8A44)";
    case 0x8A45 /* GL_UNIFORM_BLOCK_REFERENCED_BY_GEOMETRY_SHADER */:
      return "GL_UNIFORM_BLOCK_REFERENCED_BY_GEOMETRY_SHADER (0x8A45)";
    case 0x8A46 /* GL_UNIFORM_BLOCK_REFERENCED_BY_FRAGMENT_SHADER */:
      return "GL_UNIFORM_BLOCK_REFERENCED_BY_FRAGMENT_SHADER (0x8A46)";
      // case 0xFFFFFFFF /* GL_INVALID_INDEX */: return "GL_INVALID_INDEX
      // (0xFFFFFFFF)";

      /* ------------------------ GL_ARB_vertex_array_bgra
       * ----------------------- */

      // case 0x80E1 /* GL_BGRA */: return "GL_BGRA (0x80E1)";

      /* ----------------------- GL_ARB_vertex_array_object
       * ---------------------- */

      // case 0x85B5 /* GL_VERTEX_ARRAY_BINDING */: return
      // "GL_VERTEX_ARRAY_BINDING (0x85B5)";

      /* ----------------------- GL_ARB_vertex_attrib_64bit
       * ---------------------- */

      // case 0x8F46 /* GL_DOUBLE_MAT2 */: return "GL_DOUBLE_MAT2 (0x8F46)";
      // case 0x8F47 /* GL_DOUBLE_MAT3 */: return "GL_DOUBLE_MAT3 (0x8F47)";
      // case 0x8F48 /* GL_DOUBLE_MAT4 */: return "GL_DOUBLE_MAT4 (0x8F48)";
      // case 0x8FFC /* GL_DOUBLE_VEC2 */: return "GL_DOUBLE_VEC2 (0x8FFC)";
      // case 0x8FFD /* GL_DOUBLE_VEC3 */: return "GL_DOUBLE_VEC3 (0x8FFD)";
      // case 0x8FFE /* GL_DOUBLE_VEC4 */: return "GL_DOUBLE_VEC4 (0x8FFE)";

      /* ---------------------- GL_ARB_vertex_attrib_binding
       * --------------------- */

    case 0x82D4 /* GL_VERTEX_ATTRIB_BINDING */:
      return "GL_VERTEX_ATTRIB_BINDING (0x82D4)";
    case 0x82D5 /* GL_VERTEX_ATTRIB_RELATIVE_OFFSET */:
      return "GL_VERTEX_ATTRIB_RELATIVE_OFFSET (0x82D5)";
    case 0x82D6 /* GL_VERTEX_BINDING_DIVISOR */:
      return "GL_VERTEX_BINDING_DIVISOR (0x82D6)";
    case 0x82D7 /* GL_VERTEX_BINDING_OFFSET */:
      return "GL_VERTEX_BINDING_OFFSET (0x82D7)";
    case 0x82D8 /* GL_VERTEX_BINDING_STRIDE */:
      return "GL_VERTEX_BINDING_STRIDE (0x82D8)";
    case 0x82D9 /* GL_MAX_VERTEX_ATTRIB_RELATIVE_OFFSET */:
      return "GL_MAX_VERTEX_ATTRIB_RELATIVE_OFFSET (0x82D9)";
    case 0x82DA /* GL_MAX_VERTEX_ATTRIB_BINDINGS */:
      return "GL_MAX_VERTEX_ATTRIB_BINDINGS (0x82DA)"; /* --------------------------
                                                          GL_ARB_vertex_blend
                                                          --------------------------
                                                        */

    // case 0x1700 /* GL_MODELVIEW0_ARB */: return "GL_MODELVIEW0_ARB (0x1700)";
    case 0x850A /* GL_MODELVIEW1_ARB */:
      return "GL_MODELVIEW1_ARB (0x850A)";
    case 0x86A4 /* GL_MAX_VERTEX_UNITS_ARB */:
      return "GL_MAX_VERTEX_UNITS_ARB (0x86A4)";
    case 0x86A5 /* GL_ACTIVE_VERTEX_UNITS_ARB */:
      return "GL_ACTIVE_VERTEX_UNITS_ARB (0x86A5)";
    case 0x86A6 /* GL_WEIGHT_SUM_UNITY_ARB */:
      return "GL_WEIGHT_SUM_UNITY_ARB (0x86A6)";
    case 0x86A7 /* GL_VERTEX_BLEND_ARB */:
      return "GL_VERTEX_BLEND_ARB (0x86A7)";
    case 0x86A8 /* GL_CURRENT_WEIGHT_ARB */:
      return "GL_CURRENT_WEIGHT_ARB (0x86A8)";
    case 0x86A9 /* GL_WEIGHT_ARRAY_TYPE_ARB */:
      return "GL_WEIGHT_ARRAY_TYPE_ARB (0x86A9)";
    case 0x86AA /* GL_WEIGHT_ARRAY_STRIDE_ARB */:
      return "GL_WEIGHT_ARRAY_STRIDE_ARB (0x86AA)";
    case 0x86AB /* GL_WEIGHT_ARRAY_SIZE_ARB */:
      return "GL_WEIGHT_ARRAY_SIZE_ARB (0x86AB)";
    case 0x86AC /* GL_WEIGHT_ARRAY_POINTER_ARB */:
      return "GL_WEIGHT_ARRAY_POINTER_ARB (0x86AC)";
    case 0x86AD /* GL_WEIGHT_ARRAY_ARB */:
      return "GL_WEIGHT_ARRAY_ARB (0x86AD)";
    case 0x8722 /* GL_MODELVIEW2_ARB */:
      return "GL_MODELVIEW2_ARB (0x8722)";
    case 0x8723 /* GL_MODELVIEW3_ARB */:
      return "GL_MODELVIEW3_ARB (0x8723)";
    case 0x8724 /* GL_MODELVIEW4_ARB */:
      return "GL_MODELVIEW4_ARB (0x8724)";
    case 0x8725 /* GL_MODELVIEW5_ARB */:
      return "GL_MODELVIEW5_ARB (0x8725)";
    case 0x8726 /* GL_MODELVIEW6_ARB */:
      return "GL_MODELVIEW6_ARB (0x8726)";
    case 0x8727 /* GL_MODELVIEW7_ARB */:
      return "GL_MODELVIEW7_ARB (0x8727)";
    case 0x8728 /* GL_MODELVIEW8_ARB */:
      return "GL_MODELVIEW8_ARB (0x8728)";
    case 0x8729 /* GL_MODELVIEW9_ARB */:
      return "GL_MODELVIEW9_ARB (0x8729)";
    case 0x872A /* GL_MODELVIEW10_ARB */:
      return "GL_MODELVIEW10_ARB (0x872A)";
    case 0x872B /* GL_MODELVIEW11_ARB */:
      return "GL_MODELVIEW11_ARB (0x872B)";
    case 0x872C /* GL_MODELVIEW12_ARB */:
      return "GL_MODELVIEW12_ARB (0x872C)";
    case 0x872D /* GL_MODELVIEW13_ARB */:
      return "GL_MODELVIEW13_ARB (0x872D)";
    case 0x872E /* GL_MODELVIEW14_ARB */:
      return "GL_MODELVIEW14_ARB (0x872E)";
    case 0x872F /* GL_MODELVIEW15_ARB */:
      return "GL_MODELVIEW15_ARB (0x872F)";
    case 0x8730 /* GL_MODELVIEW16_ARB */:
      return "GL_MODELVIEW16_ARB (0x8730)";
    case 0x8731 /* GL_MODELVIEW17_ARB */:
      return "GL_MODELVIEW17_ARB (0x8731)";
    case 0x8732 /* GL_MODELVIEW18_ARB */:
      return "GL_MODELVIEW18_ARB (0x8732)";
    case 0x8733 /* GL_MODELVIEW19_ARB */:
      return "GL_MODELVIEW19_ARB (0x8733)";
    case 0x8734 /* GL_MODELVIEW20_ARB */:
      return "GL_MODELVIEW20_ARB (0x8734)";
    case 0x8735 /* GL_MODELVIEW21_ARB */:
      return "GL_MODELVIEW21_ARB (0x8735)";
    case 0x8736 /* GL_MODELVIEW22_ARB */:
      return "GL_MODELVIEW22_ARB (0x8736)";
    case 0x8737 /* GL_MODELVIEW23_ARB */:
      return "GL_MODELVIEW23_ARB (0x8737)";
    case 0x8738 /* GL_MODELVIEW24_ARB */:
      return "GL_MODELVIEW24_ARB (0x8738)";
    case 0x8739 /* GL_MODELVIEW25_ARB */:
      return "GL_MODELVIEW25_ARB (0x8739)";
    case 0x873A /* GL_MODELVIEW26_ARB */:
      return "GL_MODELVIEW26_ARB (0x873A)";
    case 0x873B /* GL_MODELVIEW27_ARB */:
      return "GL_MODELVIEW27_ARB (0x873B)";
    case 0x873C /* GL_MODELVIEW28_ARB */:
      return "GL_MODELVIEW28_ARB (0x873C)";
    case 0x873D /* GL_MODELVIEW29_ARB */:
      return "GL_MODELVIEW29_ARB (0x873D)";
    case 0x873E /* GL_MODELVIEW30_ARB */:
      return "GL_MODELVIEW30_ARB (0x873E)";
    case 0x873F /* GL_MODELVIEW31_ARB */:
      return "GL_MODELVIEW31_ARB (0x873F)";

    /* ---------------------- GL_ARB_vertex_buffer_object ----------------------
     */

    // case 0x8764 /* GL_BUFFER_SIZE_ARB */: return "GL_BUFFER_SIZE_ARB
    // (0x8764)"; case 0x8765 /* GL_BUFFER_USAGE_ARB */: return
    // "GL_BUFFER_USAGE_ARB (0x8765)"; case 0x8892 /* GL_ARRAY_BUFFER_ARB */:
    // return "GL_ARRAY_BUFFER_ARB (0x8892)"; case 0x8893 /*
    // GL_ELEMENT_ARRAY_BUFFER_ARB */: return "GL_ELEMENT_ARRAY_BUFFER_ARB
    // (0x8893)"; case 0x8894 /* GL_ARRAY_BUFFER_BINDING_ARB */: return
    // "GL_ARRAY_BUFFER_BINDING_ARB (0x8894)"; case 0x8895 /*
    // GL_ELEMENT_ARRAY_BUFFER_BINDING_ARB */: return
    // "GL_ELEMENT_ARRAY_BUFFER_BINDING_ARB (0x8895)"; case 0x8896 /*
    // GL_VERTEX_ARRAY_BUFFER_BINDING_ARB */: return
    // "GL_VERTEX_ARRAY_BUFFER_BINDING_ARB (0x8896)"; case 0x8897 /*
    // GL_NORMAL_ARRAY_BUFFER_BINDING_ARB */: return
    // "GL_NORMAL_ARRAY_BUFFER_BINDING_ARB (0x8897)"; case 0x8898 /*
    // GL_COLOR_ARRAY_BUFFER_BINDING_ARB */: return
    // "GL_COLOR_ARRAY_BUFFER_BINDING_ARB (0x8898)"; case 0x8899 /*
    // GL_INDEX_ARRAY_BUFFER_BINDING_ARB */: return
    // "GL_INDEX_ARRAY_BUFFER_BINDING_ARB (0x8899)"; case 0x889A /*
    // GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING_ARB */: return
    // "GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING_ARB (0x889A)"; case 0x889B /*
    // GL_EDGE_FLAG_ARRAY_BUFFER_BINDING_ARB */: return
    // "GL_EDGE_FLAG_ARRAY_BUFFER_BINDING_ARB (0x889B)"; case 0x889C /*
    // GL_SECONDARY_COLOR_ARRAY_BUFFER_BINDING_ARB */: return
    // "GL_SECONDARY_COLOR_ARRAY_BUFFER_BINDING_ARB (0x889C)"; case 0x889D /*
    // GL_FOG_COORDINATE_ARRAY_BUFFER_BINDING_ARB */: return
    // "GL_FOG_COORDINATE_ARRAY_BUFFER_BINDING_ARB (0x889D)"; case 0x889E /*
    // GL_WEIGHT_ARRAY_BUFFER_BINDING_ARB */: return
    // "GL_WEIGHT_ARRAY_BUFFER_BINDING_ARB (0x889E)"; case 0x889F /*
    // GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING_ARB */: return
    // "GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING_ARB (0x889F)"; case 0x88B8 /*
    // GL_READ_ONLY_ARB */: return "GL_READ_ONLY_ARB (0x88B8)"; case 0x88B9 /*
    // GL_WRITE_ONLY_ARB */: return "GL_WRITE_ONLY_ARB (0x88B9)"; case 0x88BA /*
    // GL_READ_WRITE_ARB */: return "GL_READ_WRITE_ARB (0x88BA)"; case 0x88BB /*
    // GL_BUFFER_ACCESS_ARB */: return "GL_BUFFER_ACCESS_ARB (0x88BB)"; case
    // 0x88BC /* GL_BUFFER_MAPPED_ARB */: return "GL_BUFFER_MAPPED_ARB (0x88BC)";
    // case 0x88BD /* GL_BUFFER_MAP_POINTER_ARB */: return
    // "GL_BUFFER_MAP_POINTER_ARB (0x88BD)"; case 0x88E0 /* GL_STREAM_DRAW_ARB
    // */: return "GL_STREAM_DRAW_ARB (0x88E0)"; case 0x88E1 /*
    // GL_STREAM_READ_ARB */: return "GL_STREAM_READ_ARB (0x88E1)"; case 0x88E2
    // /* GL_STREAM_COPY_ARB */: return "GL_STREAM_COPY_ARB (0x88E2)"; case
    // 0x88E4 /* GL_STATIC_DRAW_ARB */: return "GL_STATIC_DRAW_ARB (0x88E4)";
    // case 0x88E5 /* GL_STATIC_READ_ARB */: return "GL_STATIC_READ_ARB
    // (0x88E5)"; case 0x88E6 /* GL_STATIC_COPY_ARB */: return
    // "GL_STATIC_COPY_ARB (0x88E6)"; case 0x88E8 /* GL_DYNAMIC_DRAW_ARB */:
    // return "GL_DYNAMIC_DRAW_ARB (0x88E8)"; case 0x88E9 /* GL_DYNAMIC_READ_ARB
    // */: return "GL_DYNAMIC_READ_ARB (0x88E9)"; case 0x88EA /*
    // GL_DYNAMIC_COPY_ARB */: return "GL_DYNAMIC_COPY_ARB (0x88EA)";

    /* ------------------------- GL_ARB_vertex_program -------------------------
     */

    // case 0x8458 /* GL_COLOR_SUM_ARB */: return "GL_COLOR_SUM_ARB (0x8458)";
    case 0x8620 /* GL_VERTEX_PROGRAM_ARB */:
      return "GL_VERTEX_PROGRAM_ARB (0x8620)";
    // case 0x8622 /* GL_VERTEX_ATTRIB_ARRAY_ENABLED_ARB */: return
    // "GL_VERTEX_ATTRIB_ARRAY_ENABLED_ARB (0x8622)"; case 0x8623 /*
    // GL_VERTEX_ATTRIB_ARRAY_SIZE_ARB */: return
    // "GL_VERTEX_ATTRIB_ARRAY_SIZE_ARB (0x8623)"; case 0x8624 /*
    // GL_VERTEX_ATTRIB_ARRAY_STRIDE_ARB */: return
    // "GL_VERTEX_ATTRIB_ARRAY_STRIDE_ARB (0x8624)"; case 0x8625 /*
    // GL_VERTEX_ATTRIB_ARRAY_TYPE_ARB */: return
    // "GL_VERTEX_ATTRIB_ARRAY_TYPE_ARB (0x8625)"; case 0x8626 /*
    // GL_CURRENT_VERTEX_ATTRIB_ARB */: return "GL_CURRENT_VERTEX_ATTRIB_ARB
    // (0x8626)";
    case 0x8627 /* GL_PROGRAM_LENGTH_ARB */:
      return "GL_PROGRAM_LENGTH_ARB (0x8627)";
    case 0x8628 /* GL_PROGRAM_STRING_ARB */:
      return "GL_PROGRAM_STRING_ARB (0x8628)";
    case 0x862E /* GL_MAX_PROGRAM_MATRIX_STACK_DEPTH_ARB */:
      return "GL_MAX_PROGRAM_MATRIX_STACK_DEPTH_ARB (0x862E)";
    case 0x862F /* GL_MAX_PROGRAM_MATRICES_ARB */:
      return "GL_MAX_PROGRAM_MATRICES_ARB (0x862F)";
    case 0x8640 /* GL_CURRENT_MATRIX_STACK_DEPTH_ARB */:
      return "GL_CURRENT_MATRIX_STACK_DEPTH_ARB (0x8640)";
    case 0x8641 /* GL_CURRENT_MATRIX_ARB */:
      return "GL_CURRENT_MATRIX_ARB (0x8641)";
    // case 0x8642 /* GL_VERTEX_PROGRAM_POINT_SIZE_ARB */: return
    // "GL_VERTEX_PROGRAM_POINT_SIZE_ARB (0x8642)"; case 0x8643 /*
    // GL_VERTEX_PROGRAM_TWO_SIDE_ARB */: return "GL_VERTEX_PROGRAM_TWO_SIDE_ARB
    // (0x8643)"; case 0x8645 /* GL_VERTEX_ATTRIB_ARRAY_POINTER_ARB */: return
    // "GL_VERTEX_ATTRIB_ARRAY_POINTER_ARB (0x8645)";
    case 0x864B /* GL_PROGRAM_ERROR_POSITION_ARB */:
      return "GL_PROGRAM_ERROR_POSITION_ARB (0x864B)";
    case 0x8677 /* GL_PROGRAM_BINDING_ARB */:
      return "GL_PROGRAM_BINDING_ARB (0x8677)";
    // case 0x8869 /* GL_MAX_VERTEX_ATTRIBS_ARB */: return
    // "GL_MAX_VERTEX_ATTRIBS_ARB (0x8869)"; case 0x886A /*
    // GL_VERTEX_ATTRIB_ARRAY_NORMALIZED_ARB */: return
    // "GL_VERTEX_ATTRIB_ARRAY_NORMALIZED_ARB (0x886A)";
    case 0x8874 /* GL_PROGRAM_ERROR_STRING_ARB */:
      return "GL_PROGRAM_ERROR_STRING_ARB (0x8874)";
    case 0x8875 /* GL_PROGRAM_FORMAT_ASCII_ARB */:
      return "GL_PROGRAM_FORMAT_ASCII_ARB (0x8875)";
    case 0x8876 /* GL_PROGRAM_FORMAT_ARB */:
      return "GL_PROGRAM_FORMAT_ARB (0x8876)";
    case 0x88A4 /* GL_PROGRAM_TEMPORARIES_ARB */:
      return "GL_PROGRAM_TEMPORARIES_ARB (0x88A4)";
    case 0x88A5 /* GL_MAX_PROGRAM_TEMPORARIES_ARB */:
      return "GL_MAX_PROGRAM_TEMPORARIES_ARB (0x88A5)";
    case 0x88A6 /* GL_PROGRAM_NATIVE_TEMPORARIES_ARB */:
      return "GL_PROGRAM_NATIVE_TEMPORARIES_ARB (0x88A6)";
    case 0x88A7 /* GL_MAX_PROGRAM_NATIVE_TEMPORARIES_ARB */:
      return "GL_MAX_PROGRAM_NATIVE_TEMPORARIES_ARB (0x88A7)";
    case 0x88A8 /* GL_PROGRAM_PARAMETERS_ARB */:
      return "GL_PROGRAM_PARAMETERS_ARB (0x88A8)";
    case 0x88A9 /* GL_MAX_PROGRAM_PARAMETERS_ARB */:
      return "GL_MAX_PROGRAM_PARAMETERS_ARB (0x88A9)";
    case 0x88AA /* GL_PROGRAM_NATIVE_PARAMETERS_ARB */:
      return "GL_PROGRAM_NATIVE_PARAMETERS_ARB (0x88AA)";
    case 0x88AB /* GL_MAX_PROGRAM_NATIVE_PARAMETERS_ARB */:
      return "GL_MAX_PROGRAM_NATIVE_PARAMETERS_ARB (0x88AB)";
    case 0x88AC /* GL_PROGRAM_ATTRIBS_ARB */:
      return "GL_PROGRAM_ATTRIBS_ARB (0x88AC)";
    case 0x88AD /* GL_MAX_PROGRAM_ATTRIBS_ARB */:
      return "GL_MAX_PROGRAM_ATTRIBS_ARB (0x88AD)";
    case 0x88AE /* GL_PROGRAM_NATIVE_ATTRIBS_ARB */:
      return "GL_PROGRAM_NATIVE_ATTRIBS_ARB (0x88AE)";
    case 0x88AF /* GL_MAX_PROGRAM_NATIVE_ATTRIBS_ARB */:
      return "GL_MAX_PROGRAM_NATIVE_ATTRIBS_ARB (0x88AF)";
    case 0x88B0 /* GL_PROGRAM_ADDRESS_REGISTERS_ARB */:
      return "GL_PROGRAM_ADDRESS_REGISTERS_ARB (0x88B0)";
    case 0x88B1 /* GL_MAX_PROGRAM_ADDRESS_REGISTERS_ARB */:
      return "GL_MAX_PROGRAM_ADDRESS_REGISTERS_ARB (0x88B1)";
    case 0x88B2 /* GL_PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB */:
      return "GL_PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB (0x88B2)";
    case 0x88B3 /* GL_MAX_PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB */:
      return "GL_MAX_PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB (0x88B3)";
    case 0x88B4 /* GL_MAX_PROGRAM_LOCAL_PARAMETERS_ARB */:
      return "GL_MAX_PROGRAM_LOCAL_PARAMETERS_ARB (0x88B4)";
    case 0x88B5 /* GL_MAX_PROGRAM_ENV_PARAMETERS_ARB */:
      return "GL_MAX_PROGRAM_ENV_PARAMETERS_ARB (0x88B5)";
    case 0x88B6 /* GL_PROGRAM_UNDER_NATIVE_LIMITS_ARB */:
      return "GL_PROGRAM_UNDER_NATIVE_LIMITS_ARB (0x88B6)";
    case 0x88B7 /* GL_TRANSPOSE_CURRENT_MATRIX_ARB */:
      return "GL_TRANSPOSE_CURRENT_MATRIX_ARB (0x88B7)";
    case 0x88C0 /* GL_MATRIX0_ARB */:
      return "GL_MATRIX0_ARB (0x88C0)";
    case 0x88C1 /* GL_MATRIX1_ARB */:
      return "GL_MATRIX1_ARB (0x88C1)";
    case 0x88C2 /* GL_MATRIX2_ARB */:
      return "GL_MATRIX2_ARB (0x88C2)";
    case 0x88C3 /* GL_MATRIX3_ARB */:
      return "GL_MATRIX3_ARB (0x88C3)";
    case 0x88C4 /* GL_MATRIX4_ARB */:
      return "GL_MATRIX4_ARB (0x88C4)";
    case 0x88C5 /* GL_MATRIX5_ARB */:
      return "GL_MATRIX5_ARB (0x88C5)";
    case 0x88C6 /* GL_MATRIX6_ARB */:
      return "GL_MATRIX6_ARB (0x88C6)";
    case 0x88C7 /* GL_MATRIX7_ARB */:
      return "GL_MATRIX7_ARB (0x88C7)";
    case 0x88C8 /* GL_MATRIX8_ARB */:
      return "GL_MATRIX8_ARB (0x88C8)";
    case 0x88C9 /* GL_MATRIX9_ARB */:
      return "GL_MATRIX9_ARB (0x88C9)";
    case 0x88CA /* GL_MATRIX10_ARB */:
      return "GL_MATRIX10_ARB (0x88CA)";
    case 0x88CB /* GL_MATRIX11_ARB */:
      return "GL_MATRIX11_ARB (0x88CB)";
    case 0x88CC /* GL_MATRIX12_ARB */:
      return "GL_MATRIX12_ARB (0x88CC)";
    case 0x88CD /* GL_MATRIX13_ARB */:
      return "GL_MATRIX13_ARB (0x88CD)";
    case 0x88CE /* GL_MATRIX14_ARB */:
      return "GL_MATRIX14_ARB (0x88CE)";
    case 0x88CF /* GL_MATRIX15_ARB */:
      return "GL_MATRIX15_ARB (0x88CF)";
    case 0x88D0 /* GL_MATRIX16_ARB */:
      return "GL_MATRIX16_ARB (0x88D0)";
    case 0x88D1 /* GL_MATRIX17_ARB */:
      return "GL_MATRIX17_ARB (0x88D1)";
    case 0x88D2 /* GL_MATRIX18_ARB */:
      return "GL_MATRIX18_ARB (0x88D2)";
    case 0x88D3 /* GL_MATRIX19_ARB */:
      return "GL_MATRIX19_ARB (0x88D3)";
    case 0x88D4 /* GL_MATRIX20_ARB */:
      return "GL_MATRIX20_ARB (0x88D4)";
    case 0x88D5 /* GL_MATRIX21_ARB */:
      return "GL_MATRIX21_ARB (0x88D5)";
    case 0x88D6 /* GL_MATRIX22_ARB */:
      return "GL_MATRIX22_ARB (0x88D6)";
    case 0x88D7 /* GL_MATRIX23_ARB */:
      return "GL_MATRIX23_ARB (0x88D7)";
    case 0x88D8 /* GL_MATRIX24_ARB */:
      return "GL_MATRIX24_ARB (0x88D8)";
    case 0x88D9 /* GL_MATRIX25_ARB */:
      return "GL_MATRIX25_ARB (0x88D9)";
    case 0x88DA /* GL_MATRIX26_ARB */:
      return "GL_MATRIX26_ARB (0x88DA)";
    case 0x88DB /* GL_MATRIX27_ARB */:
      return "GL_MATRIX27_ARB (0x88DB)";
    case 0x88DC /* GL_MATRIX28_ARB */:
      return "GL_MATRIX28_ARB (0x88DC)";
    case 0x88DD /* GL_MATRIX29_ARB */:
      return "GL_MATRIX29_ARB (0x88DD)";
    case 0x88DE /* GL_MATRIX30_ARB */:
      return "GL_MATRIX30_ARB (0x88DE)";
    case 0x88DF /* GL_MATRIX31_ARB */:
      return "GL_MATRIX31_ARB (0x88DF)";

    /* -------------------------- GL_ARB_vertex_shader -------------------------
     */

    // case 0x8B31 /* GL_VERTEX_SHADER_ARB */: return "GL_VERTEX_SHADER_ARB
    // (0x8B31)"; case 0x8B4A /* GL_MAX_VERTEX_UNIFORM_COMPONENTS_ARB */: return
    // "GL_MAX_VERTEX_UNIFORM_COMPONENTS_ARB (0x8B4A)"; case 0x8B4B /*
    // GL_MAX_VARYING_FLOATS_ARB */: return "GL_MAX_VARYING_FLOATS_ARB (0x8B4B)";
    // case 0x8B4C /* GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS_ARB */: return
    // "GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS_ARB (0x8B4C)"; case 0x8B4D /*
    // GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS_ARB */: return
    // "GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS_ARB (0x8B4D)"; case 0x8B89 /*
    // GL_OBJECT_ACTIVE_ATTRIBUTES_ARB */: return
    // "GL_OBJECT_ACTIVE_ATTRIBUTES_ARB (0x8B89)"; case 0x8B8A /*
    // GL_OBJECT_ACTIVE_ATTRIBUTE_MAX_LENGTH_ARB */: return
    // "GL_OBJECT_ACTIVE_ATTRIBUTE_MAX_LENGTH_ARB (0x8B8A)";

    /* ------------------- GL_ARB_vertex_type_2_10_10_10_rev -------------------
     */

    // case 0x8368 /* GL_UNSIGNED_INT_2_10_10_10_REV */: return
    // "GL_UNSIGNED_INT_2_10_10_10_REV (0x8368)";
    case 0x8D9F /* GL_INT_2_10_10_10_REV */:
      return "GL_INT_2_10_10_10_REV (0x8D9F)";

    /* ------------------------- GL_ARB_viewport_array -------------------------
     */

    // case 0x0B70 /* GL_DEPTH_RANGE */: return "GL_DEPTH_RANGE (0x0B70)";
    // case 0x0BA2 /* GL_VIEWPORT */: return "GL_VIEWPORT (0x0BA2)";
    // case 0x0C10 /* GL_SCISSOR_BOX */: return "GL_SCISSOR_BOX (0x0C10)";
    // case 0x0C11 /* GL_SCISSOR_TEST */: return "GL_SCISSOR_TEST (0x0C11)";
    case 0x825B /* GL_MAX_VIEWPORTS */:
      return "GL_MAX_VIEWPORTS (0x825B)";
    case 0x825C /* GL_VIEWPORT_SUBPIXEL_BITS */:
      return "GL_VIEWPORT_SUBPIXEL_BITS (0x825C)";
    case 0x825D /* GL_VIEWPORT_BOUNDS_RANGE */:
      return "GL_VIEWPORT_BOUNDS_RANGE (0x825D)";
    case 0x825E /* GL_LAYER_PROVOKING_VERTEX */:
      return "GL_LAYER_PROVOKING_VERTEX (0x825E)";
    case 0x825F /* GL_VIEWPORT_INDEX_PROVOKING_VERTEX */:
      return "GL_VIEWPORT_INDEX_PROVOKING_VERTEX (0x825F)";
    case 0x8260 /* GL_UNDEFINED_VERTEX */:
      return "GL_UNDEFINED_VERTEX (0x8260)";
      // case 0x8E4D /* GL_FIRST_VERTEX_CONVENTION */: return
      // "GL_FIRST_VERTEX_CONVENTION (0x8E4D)"; case 0x8E4E /*
      // GL_LAST_VERTEX_CONVENTION */: return "GL_LAST_VERTEX_CONVENTION
      // (0x8E4E)"; case 0x8E4F /* GL_PROVOKING_VERTEX */: return
      // "GL_PROVOKING_VERTEX (0x8E4F)";

      /* --------------------------- GL_ARB_window_pos
       * --------------------------- */

      /* ------------------------- GL_ATIX_point_sprites
       * ------------------------- */

    case 0x60B0 /* GL_TEXTURE_POINT_MODE_ATIX */:
      return "GL_TEXTURE_POINT_MODE_ATIX (0x60B0)";
    case 0x60B1 /* GL_TEXTURE_POINT_ONE_COORD_ATIX */:
      return "GL_TEXTURE_POINT_ONE_COORD_ATIX (0x60B1)";
    case 0x60B2 /* GL_TEXTURE_POINT_SPRITE_ATIX */:
      return "GL_TEXTURE_POINT_SPRITE_ATIX (0x60B2)";
    case 0x60B3 /* GL_POINT_SPRITE_CULL_MODE_ATIX */:
      return "GL_POINT_SPRITE_CULL_MODE_ATIX (0x60B3)";
    case 0x60B4 /* GL_POINT_SPRITE_CULL_CENTER_ATIX */:
      return "GL_POINT_SPRITE_CULL_CENTER_ATIX (0x60B4)";
    case 0x60B5 /* GL_POINT_SPRITE_CULL_CLIP_ATIX */:
      return "GL_POINT_SPRITE_CULL_CLIP_ATIX (0x60B5)";

      /* ---------------------- GL_ATIX_texture_env_combine3
       * --------------------- */

    case 0x8744 /* GL_MODULATE_ADD_ATIX */:
      return "GL_MODULATE_ADD_ATIX (0x8744)";
    case 0x8745 /* GL_MODULATE_SIGNED_ADD_ATIX */:
      return "GL_MODULATE_SIGNED_ADD_ATIX (0x8745)";
    case 0x8746 /* GL_MODULATE_SUBTRACT_ATIX */:
      return "GL_MODULATE_SUBTRACT_ATIX (0x8746)";

      /* ----------------------- GL_ATIX_texture_env_route
       * ----------------------- */

    case 0x8747 /* GL_SECONDARY_COLOR_ATIX */:
      return "GL_SECONDARY_COLOR_ATIX (0x8747)";
    case 0x8748 /* GL_TEXTURE_OUTPUT_RGB_ATIX */:
      return "GL_TEXTURE_OUTPUT_RGB_ATIX (0x8748)";
    case 0x8749 /* GL_TEXTURE_OUTPUT_ALPHA_ATIX */:
      return "GL_TEXTURE_OUTPUT_ALPHA_ATIX (0x8749)";

      /* ---------------- GL_ATIX_vertex_shader_output_point_size
       * ---------------- */

    case 0x610E /* GL_OUTPUT_POINT_SIZE_ATIX */:
      return "GL_OUTPUT_POINT_SIZE_ATIX (0x610E)";

      /* -------------------------- GL_ATI_draw_buffers
       * -------------------------- */

      // case 0x8824 /* GL_MAX_DRAW_BUFFERS_ATI */: return
      // "GL_MAX_DRAW_BUFFERS_ATI (0x8824)"; case 0x8825 /* GL_DRAW_BUFFER0_ATI
      // */: return "GL_DRAW_BUFFER0_ATI (0x8825)"; case 0x8826 /*
      // GL_DRAW_BUFFER1_ATI */: return "GL_DRAW_BUFFER1_ATI (0x8826)"; case
      // 0x8827 /* GL_DRAW_BUFFER2_ATI */: return "GL_DRAW_BUFFER2_ATI (0x8827)";
      // case 0x8828 /* GL_DRAW_BUFFER3_ATI */: return "GL_DRAW_BUFFER3_ATI
      // (0x8828)"; case 0x8829 /* GL_DRAW_BUFFER4_ATI */: return
      // "GL_DRAW_BUFFER4_ATI (0x8829)"; case 0x882A /* GL_DRAW_BUFFER5_ATI */:
      // return "GL_DRAW_BUFFER5_ATI (0x882A)"; case 0x882B /*
      // GL_DRAW_BUFFER6_ATI */: return "GL_DRAW_BUFFER6_ATI (0x882B)"; case
      // 0x882C /* GL_DRAW_BUFFER7_ATI */: return "GL_DRAW_BUFFER7_ATI (0x882C)";
      // case 0x882D /* GL_DRAW_BUFFER8_ATI */: return "GL_DRAW_BUFFER8_ATI
      // (0x882D)"; case 0x882E /* GL_DRAW_BUFFER9_ATI */: return
      // "GL_DRAW_BUFFER9_ATI (0x882E)"; case 0x882F /* GL_DRAW_BUFFER10_ATI */:
      // return "GL_DRAW_BUFFER10_ATI (0x882F)"; case 0x8830 /*
      // GL_DRAW_BUFFER11_ATI */: return "GL_DRAW_BUFFER11_ATI (0x8830)"; case
      // 0x8831 /* GL_DRAW_BUFFER12_ATI */: return "GL_DRAW_BUFFER12_ATI
      // (0x8831)"; case 0x8832 /* GL_DRAW_BUFFER13_ATI */: return
      // "GL_DRAW_BUFFER13_ATI (0x8832)"; case 0x8833 /* GL_DRAW_BUFFER14_ATI */:
      // return "GL_DRAW_BUFFER14_ATI (0x8833)"; case 0x8834 /*
      // GL_DRAW_BUFFER15_ATI */: return "GL_DRAW_BUFFER15_ATI (0x8834)";/*
      // -------------------------- GL_ATI_element_array
      // ------------------------- */

    case 0x8768 /* GL_ELEMENT_ARRAY_ATI */:
      return "GL_ELEMENT_ARRAY_ATI (0x8768)";
    case 0x8769 /* GL_ELEMENT_ARRAY_TYPE_ATI */:
      return "GL_ELEMENT_ARRAY_TYPE_ATI (0x8769)";
    case 0x876A /* GL_ELEMENT_ARRAY_POINTER_ATI */:
      return "GL_ELEMENT_ARRAY_POINTER_ATI (0x876A)";

      /* ------------------------- GL_ATI_envmap_bumpmap
       * ------------------------- */

    case 0x8775 /* GL_BUMP_ROT_MATRIX_ATI */:
      return "GL_BUMP_ROT_MATRIX_ATI (0x8775)";
    case 0x8776 /* GL_BUMP_ROT_MATRIX_SIZE_ATI */:
      return "GL_BUMP_ROT_MATRIX_SIZE_ATI (0x8776)";
    case 0x8777 /* GL_BUMP_NUM_TEX_UNITS_ATI */:
      return "GL_BUMP_NUM_TEX_UNITS_ATI (0x8777)";
    case 0x8778 /* GL_BUMP_TEX_UNITS_ATI */:
      return "GL_BUMP_TEX_UNITS_ATI (0x8778)";
    case 0x8779 /* GL_DUDV_ATI */:
      return "GL_DUDV_ATI (0x8779)";
    case 0x877A /* GL_DU8DV8_ATI */:
      return "GL_DU8DV8_ATI (0x877A)";
    case 0x877B /* GL_BUMP_ENVMAP_ATI */:
      return "GL_BUMP_ENVMAP_ATI (0x877B)";
    case 0x877C /* GL_BUMP_TARGET_ATI */:
      return "GL_BUMP_TARGET_ATI (0x877C)";

    /* ------------------------- GL_ATI_fragment_shader ------------------------
     */

    // case 0x00000001 /* GL_RED_BIT_ATI */: return "GL_RED_BIT_ATI
    // (0x00000001)"; case 0x00000001 /* GL_2X_BIT_ATI */: return "GL_2X_BIT_ATI
    // (0x00000001)"; case 0x00000002 /* GL_4X_BIT_ATI */: return "GL_4X_BIT_ATI
    // (0x00000002)"; case 0x00000002 /* GL_GREEN_BIT_ATI */: return
    // "GL_GREEN_BIT_ATI (0x00000002)"; case 0x00000002 /* GL_COMP_BIT_ATI */:
    // return "GL_COMP_BIT_ATI (0x00000002)"; case 0x00000004 /* GL_BLUE_BIT_ATI
    // */: return "GL_BLUE_BIT_ATI (0x00000004)"; case 0x00000004 /*
    // GL_8X_BIT_ATI */: return "GL_8X_BIT_ATI (0x00000004)"; case 0x00000004 /*
    // GL_NEGATE_BIT_ATI */: return "GL_NEGATE_BIT_ATI (0x00000004)"; case
    // 0x00000008 /* GL_BIAS_BIT_ATI */: return "GL_BIAS_BIT_ATI (0x00000008)";
    // case 0x00000008 /* GL_HALF_BIT_ATI */: return "GL_HALF_BIT_ATI
    // (0x00000008)"; case 0x00000010 /* GL_QUARTER_BIT_ATI */: return
    // "GL_QUARTER_BIT_ATI (0x00000010)"; case 0x00000020 /* GL_EIGHTH_BIT_ATI
    // */: return "GL_EIGHTH_BIT_ATI (0x00000020)"; case 0x00000040 /*
    // GL_SATURATE_BIT_ATI */: return "GL_SATURATE_BIT_ATI (0x00000040)";
    case 0x8920 /* GL_FRAGMENT_SHADER_ATI */:
      return "GL_FRAGMENT_SHADER_ATI (0x8920)";
    case 0x8921 /* GL_REG_0_ATI */:
      return "GL_REG_0_ATI (0x8921)";
    case 0x8922 /* GL_REG_1_ATI */:
      return "GL_REG_1_ATI (0x8922)";
    case 0x8923 /* GL_REG_2_ATI */:
      return "GL_REG_2_ATI (0x8923)";
    case 0x8924 /* GL_REG_3_ATI */:
      return "GL_REG_3_ATI (0x8924)";
    case 0x8925 /* GL_REG_4_ATI */:
      return "GL_REG_4_ATI (0x8925)";
    case 0x8926 /* GL_REG_5_ATI */:
      return "GL_REG_5_ATI (0x8926)";
    case 0x8941 /* GL_CON_0_ATI */:
      return "GL_CON_0_ATI (0x8941)";
    case 0x8942 /* GL_CON_1_ATI */:
      return "GL_CON_1_ATI (0x8942)";
    case 0x8943 /* GL_CON_2_ATI */:
      return "GL_CON_2_ATI (0x8943)";
    case 0x8944 /* GL_CON_3_ATI */:
      return "GL_CON_3_ATI (0x8944)";
    case 0x8945 /* GL_CON_4_ATI */:
      return "GL_CON_4_ATI (0x8945)";
    case 0x8946 /* GL_CON_5_ATI */:
      return "GL_CON_5_ATI (0x8946)";
    case 0x8947 /* GL_CON_6_ATI */:
      return "GL_CON_6_ATI (0x8947)";
    case 0x8948 /* GL_CON_7_ATI */:
      return "GL_CON_7_ATI (0x8948)";
    case 0x8961 /* GL_MOV_ATI */:
      return "GL_MOV_ATI (0x8961)";
    case 0x8963 /* GL_ADD_ATI */:
      return "GL_ADD_ATI (0x8963)";
    case 0x8964 /* GL_MUL_ATI */:
      return "GL_MUL_ATI (0x8964)";
    case 0x8965 /* GL_SUB_ATI */:
      return "GL_SUB_ATI (0x8965)";
    case 0x8966 /* GL_DOT3_ATI */:
      return "GL_DOT3_ATI (0x8966)";
    case 0x8967 /* GL_DOT4_ATI */:
      return "GL_DOT4_ATI (0x8967)";
    case 0x8968 /* GL_MAD_ATI */:
      return "GL_MAD_ATI (0x8968)";
    case 0x8969 /* GL_LERP_ATI */:
      return "GL_LERP_ATI (0x8969)";
    case 0x896A /* GL_CND_ATI */:
      return "GL_CND_ATI (0x896A)";
    case 0x896B /* GL_CND0_ATI */:
      return "GL_CND0_ATI (0x896B)";
    case 0x896C /* GL_DOT2_ADD_ATI */:
      return "GL_DOT2_ADD_ATI (0x896C)";
    case 0x896D /* GL_SECONDARY_INTERPOLATOR_ATI */:
      return "GL_SECONDARY_INTERPOLATOR_ATI (0x896D)";
    case 0x896E /* GL_NUM_FRAGMENT_REGISTERS_ATI */:
      return "GL_NUM_FRAGMENT_REGISTERS_ATI (0x896E)";
    case 0x896F /* GL_NUM_FRAGMENT_CONSTANTS_ATI */:
      return "GL_NUM_FRAGMENT_CONSTANTS_ATI (0x896F)";
    case 0x8970 /* GL_NUM_PASSES_ATI */:
      return "GL_NUM_PASSES_ATI (0x8970)";
    case 0x8973 /* GL_NUM_INPUT_INTERPOLATOR_COMPONENTS_ATI */:
      return "GL_NUM_INPUT_INTERPOLATOR_COMPONENTS_ATI (0x8973)";
    case 0x8974 /* GL_NUM_LOOPBACK_COMPONENTS_ATI */:
      return "GL_NUM_LOOPBACK_COMPONENTS_ATI (0x8974)";
    case 0x8975 /* GL_COLOR_ALPHA_PAIRING_ATI */:
      return "GL_COLOR_ALPHA_PAIRING_ATI (0x8975)";
    case 0x8976 /* GL_SWIZZLE_STR_ATI */:
      return "GL_SWIZZLE_STR_ATI (0x8976)";
    case 0x8977 /* GL_SWIZZLE_STQ_ATI */:
      return "GL_SWIZZLE_STQ_ATI (0x8977)";
    case 0x8978 /* GL_SWIZZLE_STR_DR_ATI */:
      return "GL_SWIZZLE_STR_DR_ATI (0x8978)";
    case 0x8979 /* GL_SWIZZLE_STQ_DQ_ATI */:
      return "GL_SWIZZLE_STQ_DQ_ATI (0x8979)";
    case 0x897A /* GL_SWIZZLE_STRQ_ATI */:
      return "GL_SWIZZLE_STRQ_ATI (0x897A)";
    case 0x897B /* GL_SWIZZLE_STRQ_DQ_ATI */:
      return "GL_SWIZZLE_STRQ_DQ_ATI (0x897B)";

      /* ------------------------ GL_ATI_map_object_buffer
       * ----------------------- */

      /* ----------------------------- GL_ATI_meminfo
       * ---------------------------- */

    case 0x87FB /* GL_VBO_FREE_MEMORY_ATI */:
      return "GL_VBO_FREE_MEMORY_ATI (0x87FB)";
    case 0x87FC /* GL_TEXTURE_FREE_MEMORY_ATI */:
      return "GL_TEXTURE_FREE_MEMORY_ATI (0x87FC)";
    case 0x87FD /* GL_RENDERBUFFER_FREE_MEMORY_ATI */:
      return "GL_RENDERBUFFER_FREE_MEMORY_ATI (0x87FD)";

      /* -------------------------- GL_ATI_pn_triangles
       * -------------------------- */

    case 0x87F0 /* GL_PN_TRIANGLES_ATI */:
      return "GL_PN_TRIANGLES_ATI (0x87F0)";
    case 0x87F1 /* GL_MAX_PN_TRIANGLES_TESSELATION_LEVEL_ATI */:
      return "GL_MAX_PN_TRIANGLES_TESSELATION_LEVEL_ATI (0x87F1)";
    case 0x87F2 /* GL_PN_TRIANGLES_POINT_MODE_ATI */:
      return "GL_PN_TRIANGLES_POINT_MODE_ATI (0x87F2)";
    case 0x87F3 /* GL_PN_TRIANGLES_NORMAL_MODE_ATI */:
      return "GL_PN_TRIANGLES_NORMAL_MODE_ATI (0x87F3)";
    case 0x87F4 /* GL_PN_TRIANGLES_TESSELATION_LEVEL_ATI */:
      return "GL_PN_TRIANGLES_TESSELATION_LEVEL_ATI (0x87F4)";
    case 0x87F5 /* GL_PN_TRIANGLES_POINT_MODE_LINEAR_ATI */:
      return "GL_PN_TRIANGLES_POINT_MODE_LINEAR_ATI (0x87F5)";
    case 0x87F6 /* GL_PN_TRIANGLES_POINT_MODE_CUBIC_ATI */:
      return "GL_PN_TRIANGLES_POINT_MODE_CUBIC_ATI (0x87F6)";
    case 0x87F7 /* GL_PN_TRIANGLES_NORMAL_MODE_LINEAR_ATI */:
      return "GL_PN_TRIANGLES_NORMAL_MODE_LINEAR_ATI (0x87F7)";
    case 0x87F8 /* GL_PN_TRIANGLES_NORMAL_MODE_QUADRATIC_ATI */:
      return "GL_PN_TRIANGLES_NORMAL_MODE_QUADRATIC_ATI (0x87F8)";

      /* ------------------------ GL_ATI_separate_stencil
       * ------------------------ */

      // case 0x8800 /* GL_STENCIL_BACK_FUNC_ATI */: return
      // "GL_STENCIL_BACK_FUNC_ATI (0x8800)"; case 0x8801 /*
      // GL_STENCIL_BACK_FAIL_ATI */: return "GL_STENCIL_BACK_FAIL_ATI (0x8801)";
      // case 0x8802 /* GL_STENCIL_BACK_PASS_DEPTH_FAIL_ATI */: return
      // "GL_STENCIL_BACK_PASS_DEPTH_FAIL_ATI (0x8802)"; case 0x8803 /*
      // GL_STENCIL_BACK_PASS_DEPTH_PASS_ATI */: return
      // "GL_STENCIL_BACK_PASS_DEPTH_PASS_ATI (0x8803)";

      /* ----------------------- GL_ATI_shader_texture_lod
       * ----------------------- */

      /* ---------------------- GL_ATI_text_fragment_shader
       * ---------------------- */

    case 0x8200 /* GL_TEXT_FRAGMENT_SHADER_ATI */:
      return "GL_TEXT_FRAGMENT_SHADER_ATI (0x8200)";

      /* --------------------- GL_ATI_texture_compression_3dc
       * -------------------- */

    case 0x8837 /* GL_COMPRESSED_LUMINANCE_ALPHA_3DC_ATI */:
      return "GL_COMPRESSED_LUMINANCE_ALPHA_3DC_ATI (0x8837)";

      /* ---------------------- GL_ATI_texture_env_combine3
       * ---------------------- */

      // case 0x8744 /* GL_MODULATE_ADD_ATI */: return "GL_MODULATE_ADD_ATI
      // (0x8744)"; case 0x8745 /* GL_MODULATE_SIGNED_ADD_ATI */: return
      // "GL_MODULATE_SIGNED_ADD_ATI (0x8745)"; case 0x8746 /*
      // GL_MODULATE_SUBTRACT_ATI */: return "GL_MODULATE_SUBTRACT_ATI (0x8746)";

      /* -------------------------- GL_ATI_texture_float
       * ------------------------- */

      // case 0x8814 /* GL_RGBA_FLOAT32_ATI */: return "GL_RGBA_FLOAT32_ATI
      // (0x8814)"; case 0x8815 /* GL_RGB_FLOAT32_ATI */: return
      // "GL_RGB_FLOAT32_ATI (0x8815)"; case 0x8816 /* GL_ALPHA_FLOAT32_ATI */:
      // return "GL_ALPHA_FLOAT32_ATI (0x8816)"; case 0x8817 /*
      // GL_INTENSITY_FLOAT32_ATI */: return "GL_INTENSITY_FLOAT32_ATI (0x8817)";
      // case 0x8818 /* GL_LUMINANCE_FLOAT32_ATI */: return
      // "GL_LUMINANCE_FLOAT32_ATI (0x8818)"; case 0x8819 /*
      // GL_LUMINANCE_ALPHA_FLOAT32_ATI */: return
      // "GL_LUMINANCE_ALPHA_FLOAT32_ATI (0x8819)"; case 0x881A /*
      // GL_RGBA_FLOAT16_ATI */: return "GL_RGBA_FLOAT16_ATI (0x881A)"; case
      // 0x881B /* GL_RGB_FLOAT16_ATI */: return "GL_RGB_FLOAT16_ATI (0x881B)";
      // case 0x881C /* GL_ALPHA_FLOAT16_ATI */: return "GL_ALPHA_FLOAT16_ATI
      // (0x881C)"; case 0x881D /* GL_INTENSITY_FLOAT16_ATI */: return
      // "GL_INTENSITY_FLOAT16_ATI (0x881D)"; case 0x881E /*
      // GL_LUMINANCE_FLOAT16_ATI */: return "GL_LUMINANCE_FLOAT16_ATI (0x881E)";
      // case 0x881F /* GL_LUMINANCE_ALPHA_FLOAT16_ATI */: return
      // "GL_LUMINANCE_ALPHA_FLOAT16_ATI (0x881F)";

      /* ----------------------- GL_ATI_texture_mirror_once
       * ---------------------- */

    case 0x8742 /* GL_MIRROR_CLAMP_ATI */:
      return "GL_MIRROR_CLAMP_ATI (0x8742)";
    case 0x8743 /* GL_MIRROR_CLAMP_TO_EDGE_ATI */:
      return "GL_MIRROR_CLAMP_TO_EDGE_ATI (0x8743)";

      /* ----------------------- GL_ATI_vertex_array_object
       * ---------------------- */

    case 0x8760 /* GL_STATIC_ATI */:
      return "GL_STATIC_ATI (0x8760)";
    case 0x8761 /* GL_DYNAMIC_ATI */:
      return "GL_DYNAMIC_ATI (0x8761)";
    case 0x8762 /* GL_PRESERVE_ATI */:
      return "GL_PRESERVE_ATI (0x8762)";
    case 0x8763 /* GL_DISCARD_ATI */:
      return "GL_DISCARD_ATI (0x8763)";
    // case 0x8764 /* GL_OBJECT_BUFFER_SIZE_ATI */: return
    // "GL_OBJECT_BUFFER_SIZE_ATI (0x8764)"; case 0x8765 /*
    // GL_OBJECT_BUFFER_USAGE_ATI */: return "GL_OBJECT_BUFFER_USAGE_ATI
    // (0x8765)";
    case 0x8766 /* GL_ARRAY_OBJECT_BUFFER_ATI */:
      return "GL_ARRAY_OBJECT_BUFFER_ATI (0x8766)";
    case 0x8767 /* GL_ARRAY_OBJECT_OFFSET_ATI */:
      return "GL_ARRAY_OBJECT_OFFSET_ATI (0x8767)";

      /* ------------------- GL_ATI_vertex_attrib_array_object
       * ------------------- */

      /* ------------------------- GL_ATI_vertex_streams
       * ------------------------- */

    case 0x876B /* GL_MAX_VERTEX_STREAMS_ATI */:
      return "GL_MAX_VERTEX_STREAMS_ATI (0x876B)";
    case 0x876C /* GL_VERTEX_SOURCE_ATI */:
      return "GL_VERTEX_SOURCE_ATI (0x876C)";
    case 0x876D /* GL_VERTEX_STREAM0_ATI */:
      return "GL_VERTEX_STREAM0_ATI (0x876D)";
    case 0x876E /* GL_VERTEX_STREAM1_ATI */:
      return "GL_VERTEX_STREAM1_ATI (0x876E)";
    case 0x876F /* GL_VERTEX_STREAM2_ATI */:
      return "GL_VERTEX_STREAM2_ATI (0x876F)";
    case 0x8770 /* GL_VERTEX_STREAM3_ATI */:
      return "GL_VERTEX_STREAM3_ATI (0x8770)";
    case 0x8771 /* GL_VERTEX_STREAM4_ATI */:
      return "GL_VERTEX_STREAM4_ATI (0x8771)";
    case 0x8772 /* GL_VERTEX_STREAM5_ATI */:
      return "GL_VERTEX_STREAM5_ATI (0x8772)";
    case 0x8773 /* GL_VERTEX_STREAM6_ATI */:
      return "GL_VERTEX_STREAM6_ATI (0x8773)";
    case 0x8774 /* GL_VERTEX_STREAM7_ATI */:
      return "GL_VERTEX_STREAM7_ATI (0x8774)";

      /* --------------------------- GL_EXT_422_pixels
       * --------------------------- */

    case 0x80CC /* GL_422_EXT */:
      return "GL_422_EXT (0x80CC)";
    case 0x80CD /* GL_422_REV_EXT */:
      return "GL_422_REV_EXT (0x80CD)";
    case 0x80CE /* GL_422_AVERAGE_EXT */:
      return "GL_422_AVERAGE_EXT (0x80CE)";
    case 0x80CF /* GL_422_REV_AVERAGE_EXT */:
      return "GL_422_REV_AVERAGE_EXT (0x80CF)";

      /* ---------------------------- GL_EXT_Cg_shader
       * --------------------------- */

    case 0x890E /* GL_CG_VERTEX_SHADER_EXT */:
      return "GL_CG_VERTEX_SHADER_EXT (0x890E)";
    case 0x890F /* GL_CG_FRAGMENT_SHADER_EXT */:
      return "GL_CG_FRAGMENT_SHADER_EXT (0x890F)";

      /* ------------------------------ GL_EXT_abgr
       * ------------------------------ */

      // case 0x8000 /* GL_ABGR_EXT */: return "GL_ABGR_EXT (0x8000)";

      /* ------------------------------ GL_EXT_bgra
       * ------------------------------ */

      // case 0x80E0 /* GL_BGR_EXT */: return "GL_BGR_EXT (0x80E0)";
      // case 0x80E1 /* GL_BGRA_EXT */: return "GL_BGRA_EXT (0x80E1)";

      /* ------------------------ GL_EXT_bindable_uniform
       * ------------------------ */

    case 0x8DE2 /* GL_MAX_VERTEX_BINDABLE_UNIFORMS_EXT */:
      return "GL_MAX_VERTEX_BINDABLE_UNIFORMS_EXT (0x8DE2)";
    case 0x8DE3 /* GL_MAX_FRAGMENT_BINDABLE_UNIFORMS_EXT */:
      return "GL_MAX_FRAGMENT_BINDABLE_UNIFORMS_EXT (0x8DE3)";
    case 0x8DE4 /* GL_MAX_GEOMETRY_BINDABLE_UNIFORMS_EXT */:
      return "GL_MAX_GEOMETRY_BINDABLE_UNIFORMS_EXT (0x8DE4)";
    case 0x8DED /* GL_MAX_BINDABLE_UNIFORM_SIZE_EXT */:
      return "GL_MAX_BINDABLE_UNIFORM_SIZE_EXT (0x8DED)";
    case 0x8DEE /* GL_UNIFORM_BUFFER_EXT */:
      return "GL_UNIFORM_BUFFER_EXT (0x8DEE)";
    case 0x8DEF /* GL_UNIFORM_BUFFER_BINDING_EXT */:
      return "GL_UNIFORM_BUFFER_BINDING_EXT (0x8DEF)";

      /* --------------------------- GL_EXT_blend_color
       * -------------------------- */

      // case 0x8001 /* GL_CONSTANT_COLOR_EXT */: return "GL_CONSTANT_COLOR_EXT
      // (0x8001)"; case 0x8002 /* GL_ONE_MINUS_CONSTANT_COLOR_EXT */: return
      // "GL_ONE_MINUS_CONSTANT_COLOR_EXT (0x8002)"; case 0x8003 /*
      // GL_CONSTANT_ALPHA_EXT */: return "GL_CONSTANT_ALPHA_EXT (0x8003)"; case
      // 0x8004 /* GL_ONE_MINUS_CONSTANT_ALPHA_EXT */: return
      // "GL_ONE_MINUS_CONSTANT_ALPHA_EXT (0x8004)"; case 0x8005 /*
      // GL_BLEND_COLOR_EXT */: return "GL_BLEND_COLOR_EXT (0x8005)";/*
      // --------------------- GL_EXT_blend_equation_separate
      // -------------------- */

      // case 0x8009 /* GL_BLEND_EQUATION_RGB_EXT */: return
      // "GL_BLEND_EQUATION_RGB_EXT (0x8009)"; case 0x883D /*
      // GL_BLEND_EQUATION_ALPHA_EXT */: return "GL_BLEND_EQUATION_ALPHA_EXT
      // (0x883D)";/* ----------------------- GL_EXT_blend_func_separate
      // ---------------------- */

      // case 0x80C8 /* GL_BLEND_DST_RGB_EXT */: return "GL_BLEND_DST_RGB_EXT
      // (0x80C8)"; case 0x80C9 /* GL_BLEND_SRC_RGB_EXT */: return
      // "GL_BLEND_SRC_RGB_EXT (0x80C9)"; case 0x80CA /* GL_BLEND_DST_ALPHA_EXT
      // */: return "GL_BLEND_DST_ALPHA_EXT (0x80CA)"; case 0x80CB /*
      // GL_BLEND_SRC_ALPHA_EXT */: return "GL_BLEND_SRC_ALPHA_EXT (0x80CB)";/*
      // ------------------------- GL_EXT_blend_logic_op
      // ------------------------- */

      /* -------------------------- GL_EXT_blend_minmax
       * -------------------------- */

      // case 0x8006 /* GL_FUNC_ADD_EXT */: return "GL_FUNC_ADD_EXT (0x8006)";
      // case 0x8007 /* GL_MIN_EXT */: return "GL_MIN_EXT (0x8007)";
      // case 0x8008 /* GL_MAX_EXT */: return "GL_MAX_EXT (0x8008)";
      // case 0x8009 /* GL_BLEND_EQUATION_EXT */: return "GL_BLEND_EQUATION_EXT
      // (0x8009)";/* ------------------------- GL_EXT_blend_subtract
      // ------------------------- */

      // case 0x800A /* GL_FUNC_SUBTRACT_EXT */: return "GL_FUNC_SUBTRACT_EXT
      // (0x800A)"; case 0x800B /* GL_FUNC_REVERSE_SUBTRACT_EXT */: return
      // "GL_FUNC_REVERSE_SUBTRACT_EXT (0x800B)";

      /* ------------------------ GL_EXT_clip_volume_hint
       * ------------------------ */

    case 0x80F0 /* GL_CLIP_VOLUME_CLIPPING_HINT_EXT */:
      return "GL_CLIP_VOLUME_CLIPPING_HINT_EXT (0x80F0)";

      /* ------------------------------ GL_EXT_cmyka
       * ----------------------------- */

    case 0x800C /* GL_CMYK_EXT */:
      return "GL_CMYK_EXT (0x800C)";
    case 0x800D /* GL_CMYKA_EXT */:
      return "GL_CMYKA_EXT (0x800D)";
    case 0x800E /* GL_PACK_CMYK_HINT_EXT */:
      return "GL_PACK_CMYK_HINT_EXT (0x800E)";
    case 0x800F /* GL_UNPACK_CMYK_HINT_EXT */:
      return "GL_UNPACK_CMYK_HINT_EXT (0x800F)";

      /* ------------------------- GL_EXT_color_subtable
       * ------------------------- */

      /* ---------------------- GL_EXT_compiled_vertex_array
       * --------------------- */

    case 0x81A8 /* GL_ARRAY_ELEMENT_LOCK_FIRST_EXT */:
      return "GL_ARRAY_ELEMENT_LOCK_FIRST_EXT (0x81A8)";
    case 0x81A9 /* GL_ARRAY_ELEMENT_LOCK_COUNT_EXT */:
      return "GL_ARRAY_ELEMENT_LOCK_COUNT_EXT (0x81A9)";

      /* --------------------------- GL_EXT_convolution
       * -------------------------- */

      // case 0x8010 /* GL_CONVOLUTION_1D_EXT */: return "GL_CONVOLUTION_1D_EXT
      // (0x8010)"; case 0x8011 /* GL_CONVOLUTION_2D_EXT */: return
      // "GL_CONVOLUTION_2D_EXT (0x8011)"; case 0x8012 /* GL_SEPARABLE_2D_EXT */:
      // return "GL_SEPARABLE_2D_EXT (0x8012)"; case 0x8013 /*
      // GL_CONVOLUTION_BORDER_MODE_EXT */: return
      // "GL_CONVOLUTION_BORDER_MODE_EXT (0x8013)"; case 0x8014 /*
      // GL_CONVOLUTION_FILTER_SCALE_EXT */: return
      // "GL_CONVOLUTION_FILTER_SCALE_EXT (0x8014)"; case 0x8015 /*
      // GL_CONVOLUTION_FILTER_BIAS_EXT */: return
      // "GL_CONVOLUTION_FILTER_BIAS_EXT (0x8015)"; case 0x8016 /* GL_REDUCE_EXT
      // */: return "GL_REDUCE_EXT (0x8016)"; case 0x8017 /*
      // GL_CONVOLUTION_FORMAT_EXT */: return "GL_CONVOLUTION_FORMAT_EXT
      // (0x8017)"; case 0x8018 /* GL_CONVOLUTION_WIDTH_EXT */: return
      // "GL_CONVOLUTION_WIDTH_EXT (0x8018)"; case 0x8019 /*
      // GL_CONVOLUTION_HEIGHT_EXT */: return "GL_CONVOLUTION_HEIGHT_EXT
      // (0x8019)"; case 0x801A /* GL_MAX_CONVOLUTION_WIDTH_EXT */: return
      // "GL_MAX_CONVOLUTION_WIDTH_EXT (0x801A)"; case 0x801B /*
      // GL_MAX_CONVOLUTION_HEIGHT_EXT */: return "GL_MAX_CONVOLUTION_HEIGHT_EXT
      // (0x801B)"; case 0x801C /* GL_POST_CONVOLUTION_RED_SCALE_EXT */: return
      // "GL_POST_CONVOLUTION_RED_SCALE_EXT (0x801C)"; case 0x801D /*
      // GL_POST_CONVOLUTION_GREEN_SCALE_EXT */: return
      // "GL_POST_CONVOLUTION_GREEN_SCALE_EXT (0x801D)"; case 0x801E /*
      // GL_POST_CONVOLUTION_BLUE_SCALE_EXT */: return
      // "GL_POST_CONVOLUTION_BLUE_SCALE_EXT (0x801E)"; case 0x801F /*
      // GL_POST_CONVOLUTION_ALPHA_SCALE_EXT */: return
      // "GL_POST_CONVOLUTION_ALPHA_SCALE_EXT (0x801F)"; case 0x8020 /*
      // GL_POST_CONVOLUTION_RED_BIAS_EXT */: return
      // "GL_POST_CONVOLUTION_RED_BIAS_EXT (0x8020)"; case 0x8021 /*
      // GL_POST_CONVOLUTION_GREEN_BIAS_EXT */: return
      // "GL_POST_CONVOLUTION_GREEN_BIAS_EXT (0x8021)"; case 0x8022 /*
      // GL_POST_CONVOLUTION_BLUE_BIAS_EXT */: return
      // "GL_POST_CONVOLUTION_BLUE_BIAS_EXT (0x8022)"; case 0x8023 /*
      // GL_POST_CONVOLUTION_ALPHA_BIAS_EXT */: return
      // "GL_POST_CONVOLUTION_ALPHA_BIAS_EXT (0x8023)";

      /* ------------------------ GL_EXT_coordinate_frame
       * ------------------------ */

    case 0x8439 /* GL_TANGENT_ARRAY_EXT */:
      return "GL_TANGENT_ARRAY_EXT (0x8439)";
    case 0x843A /* GL_BINORMAL_ARRAY_EXT */:
      return "GL_BINORMAL_ARRAY_EXT (0x843A)";
    case 0x843B /* GL_CURRENT_TANGENT_EXT */:
      return "GL_CURRENT_TANGENT_EXT (0x843B)";
    case 0x843C /* GL_CURRENT_BINORMAL_EXT */:
      return "GL_CURRENT_BINORMAL_EXT (0x843C)";
    case 0x843E /* GL_TANGENT_ARRAY_TYPE_EXT */:
      return "GL_TANGENT_ARRAY_TYPE_EXT (0x843E)";
    case 0x843F /* GL_TANGENT_ARRAY_STRIDE_EXT */:
      return "GL_TANGENT_ARRAY_STRIDE_EXT (0x843F)";
    case 0x8440 /* GL_BINORMAL_ARRAY_TYPE_EXT */:
      return "GL_BINORMAL_ARRAY_TYPE_EXT (0x8440)";
    case 0x8441 /* GL_BINORMAL_ARRAY_STRIDE_EXT */:
      return "GL_BINORMAL_ARRAY_STRIDE_EXT (0x8441)";
    case 0x8442 /* GL_TANGENT_ARRAY_POINTER_EXT */:
      return "GL_TANGENT_ARRAY_POINTER_EXT (0x8442)";
    case 0x8443 /* GL_BINORMAL_ARRAY_POINTER_EXT */:
      return "GL_BINORMAL_ARRAY_POINTER_EXT (0x8443)";
    case 0x8444 /* GL_MAP1_TANGENT_EXT */:
      return "GL_MAP1_TANGENT_EXT (0x8444)";
    case 0x8445 /* GL_MAP2_TANGENT_EXT */:
      return "GL_MAP2_TANGENT_EXT (0x8445)";
    case 0x8446 /* GL_MAP1_BINORMAL_EXT */:
      return "GL_MAP1_BINORMAL_EXT (0x8446)";
    case 0x8447 /* GL_MAP2_BINORMAL_EXT */:
      return "GL_MAP2_BINORMAL_EXT (0x8447)";

      /* -------------------------- GL_EXT_copy_texture
       * -------------------------- */
      /* --------------------------- GL_EXT_cull_vertex
       * -------------------------- */

    case 0x81AA /* GL_CULL_VERTEX_EXT */:
      return "GL_CULL_VERTEX_EXT (0x81AA)";
    case 0x81AB /* GL_CULL_VERTEX_EYE_POSITION_EXT */:
      return "GL_CULL_VERTEX_EYE_POSITION_EXT (0x81AB)";
    case 0x81AC /* GL_CULL_VERTEX_OBJECT_POSITION_EXT */:
      return "GL_CULL_VERTEX_OBJECT_POSITION_EXT (0x81AC)";

      /* -------------------------- GL_EXT_debug_marker
       * -------------------------- */

      /* ------------------------ GL_EXT_depth_bounds_test
       * ----------------------- */

    case 0x8890 /* GL_DEPTH_BOUNDS_TEST_EXT */:
      return "GL_DEPTH_BOUNDS_TEST_EXT (0x8890)";
    case 0x8891 /* GL_DEPTH_BOUNDS_EXT */:
      return "GL_DEPTH_BOUNDS_EXT (0x8891)"; /* -----------------------
                                                GL_EXT_direct_state_access
                                                ---------------------- */

    case 0x8E2D /* GL_PROGRAM_MATRIX_EXT */:
      return "GL_PROGRAM_MATRIX_EXT (0x8E2D)";
    case 0x8E2E /* GL_TRANSPOSE_PROGRAM_MATRIX_EXT */:
      return "GL_TRANSPOSE_PROGRAM_MATRIX_EXT (0x8E2E)";
    case 0x8E2F /* GL_PROGRAM_MATRIX_STACK_DEPTH_EXT */:
      return "GL_PROGRAM_MATRIX_STACK_DEPTH_EXT (0x8E2F)";

      /* -------------------------- GL_EXT_draw_buffers2
       * ------------------------- */

      /* ------------------------- GL_EXT_draw_instanced
       * ------------------------- */

      /* ----------------------- GL_EXT_draw_range_elements
       * ---------------------- */

      // case 0x80E8 /* GL_MAX_ELEMENTS_VERTICES_EXT */: return
      // "GL_MAX_ELEMENTS_VERTICES_EXT (0x80E8)"; case 0x80E9 /*
      // GL_MAX_ELEMENTS_INDICES_EXT */: return "GL_MAX_ELEMENTS_INDICES_EXT
      // (0x80E9)";/* ---------------------------- GL_EXT_fog_coord
      // --------------------------- */

      // case 0x8450 /* GL_FOG_COORDINATE_SOURCE_EXT */: return
      // "GL_FOG_COORDINATE_SOURCE_EXT (0x8450)"; case 0x8451 /*
      // GL_FOG_COORDINATE_EXT */: return "GL_FOG_COORDINATE_EXT (0x8451)"; case
      // 0x8452 /* GL_FRAGMENT_DEPTH_EXT */: return "GL_FRAGMENT_DEPTH_EXT
      // (0x8452)"; case 0x8453 /* GL_CURRENT_FOG_COORDINATE_EXT */: return
      // "GL_CURRENT_FOG_COORDINATE_EXT (0x8453)"; case 0x8454 /*
      // GL_FOG_COORDINATE_ARRAY_TYPE_EXT */: return
      // "GL_FOG_COORDINATE_ARRAY_TYPE_EXT (0x8454)"; case 0x8455 /*
      // GL_FOG_COORDINATE_ARRAY_STRIDE_EXT */: return
      // "GL_FOG_COORDINATE_ARRAY_STRIDE_EXT (0x8455)"; case 0x8456 /*
      // GL_FOG_COORDINATE_ARRAY_POINTER_EXT */: return
      // "GL_FOG_COORDINATE_ARRAY_POINTER_EXT (0x8456)"; case 0x8457 /*
      // GL_FOG_COORDINATE_ARRAY_EXT */: return "GL_FOG_COORDINATE_ARRAY_EXT
      // (0x8457)";

      /* ------------------------ GL_EXT_fragment_lighting
       * ----------------------- */

    case 0x8400 /* GL_FRAGMENT_LIGHTING_EXT */:
      return "GL_FRAGMENT_LIGHTING_EXT (0x8400)";
    case 0x8401 /* GL_FRAGMENT_COLOR_MATERIAL_EXT */:
      return "GL_FRAGMENT_COLOR_MATERIAL_EXT (0x8401)";
    case 0x8402 /* GL_FRAGMENT_COLOR_MATERIAL_FACE_EXT */:
      return "GL_FRAGMENT_COLOR_MATERIAL_FACE_EXT (0x8402)";
    case 0x8403 /* GL_FRAGMENT_COLOR_MATERIAL_PARAMETER_EXT */:
      return "GL_FRAGMENT_COLOR_MATERIAL_PARAMETER_EXT (0x8403)";
    case 0x8404 /* GL_MAX_FRAGMENT_LIGHTS_EXT */:
      return "GL_MAX_FRAGMENT_LIGHTS_EXT (0x8404)";
    case 0x8405 /* GL_MAX_ACTIVE_LIGHTS_EXT */:
      return "GL_MAX_ACTIVE_LIGHTS_EXT (0x8405)";
    case 0x8406 /* GL_CURRENT_RASTER_NORMAL_EXT */:
      return "GL_CURRENT_RASTER_NORMAL_EXT (0x8406)";
    case 0x8407 /* GL_LIGHT_ENV_MODE_EXT */:
      return "GL_LIGHT_ENV_MODE_EXT (0x8407)";
    case 0x8408 /* GL_FRAGMENT_LIGHT_MODEL_LOCAL_VIEWER_EXT */:
      return "GL_FRAGMENT_LIGHT_MODEL_LOCAL_VIEWER_EXT (0x8408)";
    case 0x8409 /* GL_FRAGMENT_LIGHT_MODEL_TWO_SIDE_EXT */:
      return "GL_FRAGMENT_LIGHT_MODEL_TWO_SIDE_EXT (0x8409)";
    case 0x840A /* GL_FRAGMENT_LIGHT_MODEL_AMBIENT_EXT */:
      return "GL_FRAGMENT_LIGHT_MODEL_AMBIENT_EXT (0x840A)";
    case 0x840B /* GL_FRAGMENT_LIGHT_MODEL_NORMAL_INTERPOLATION_EXT */:
      return "GL_FRAGMENT_LIGHT_MODEL_NORMAL_INTERPOLATION_EXT (0x840B)";
    case 0x840C /* GL_FRAGMENT_LIGHT0_EXT */:
      return "GL_FRAGMENT_LIGHT0_EXT (0x840C)";
    case 0x8413 /* GL_FRAGMENT_LIGHT7_EXT */:
      return "GL_FRAGMENT_LIGHT7_EXT (0x8413)";

      /* ------------------------ GL_EXT_framebuffer_blit
       * ------------------------ */

      // case 0x8CA6 /* GL_DRAW_FRAMEBUFFER_BINDING_EXT */: return
      // "GL_DRAW_FRAMEBUFFER_BINDING_EXT (0x8CA6)"; case 0x8CA8 /*
      // GL_READ_FRAMEBUFFER_EXT */: return "GL_READ_FRAMEBUFFER_EXT (0x8CA8)";
      // case 0x8CA9 /* GL_DRAW_FRAMEBUFFER_EXT */: return
      // "GL_DRAW_FRAMEBUFFER_EXT (0x8CA9)"; case 0x8CAA /*
      // GL_READ_FRAMEBUFFER_BINDING_EXT */: return
      // "GL_READ_FRAMEBUFFER_BINDING_EXT (0x8CAA)";/* ---------------------
      // GL_EXT_framebuffer_multisample -------------------- */

      // case 0x8CAB /* GL_RENDERBUFFER_SAMPLES_EXT */: return
      // "GL_RENDERBUFFER_SAMPLES_EXT (0x8CAB)"; case 0x8D56 /*
      // GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_EXT */: return
      // "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_EXT (0x8D56)"; case 0x8D57 /*
      // GL_MAX_SAMPLES_EXT */: return "GL_MAX_SAMPLES_EXT (0x8D57)";/*
      // --------------- GL_EXT_framebuffer_multisample_blit_scaled
      // -------------- */

    case 0x90BA /* GL_SCALED_RESOLVE_FASTEST_EXT */:
      return "GL_SCALED_RESOLVE_FASTEST_EXT (0x90BA)";
    case 0x90BB /* GL_SCALED_RESOLVE_NICEST_EXT */:
      return "GL_SCALED_RESOLVE_NICEST_EXT (0x90BB)";

    /* ----------------------- GL_EXT_framebuffer_object -----------------------
     */

    // case 0x0506 /* GL_INVALID_FRAMEBUFFER_OPERATION_EXT */: return
    // "GL_INVALID_FRAMEBUFFER_OPERATION_EXT (0x0506)"; case 0x84E8 /*
    // GL_MAX_RENDERBUFFER_SIZE_EXT */: return "GL_MAX_RENDERBUFFER_SIZE_EXT
    // (0x84E8)"; case 0x8CA6 /* GL_FRAMEBUFFER_BINDING_EXT */: return
    // "GL_FRAMEBUFFER_BINDING_EXT (0x8CA6)"; case 0x8CA7 /*
    // GL_RENDERBUFFER_BINDING_EXT */: return "GL_RENDERBUFFER_BINDING_EXT
    // (0x8CA7)"; case 0x8CD0 /* GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE_EXT */:
    // return "GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE_EXT (0x8CD0)"; case 0x8CD1
    // /* GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME_EXT */: return
    // "GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME_EXT (0x8CD1)"; case 0x8CD2 /*
    // GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL_EXT */: return
    // "GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL_EXT (0x8CD2)"; case 0x8CD3 /*
    // GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE_EXT */: return
    // "GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE_EXT (0x8CD3)"; case
    // 0x8CD4 /* GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_3D_ZOFFSET_EXT */: return
    // "GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_3D_ZOFFSET_EXT (0x8CD4)"; case 0x8CD5
    // /* GL_FRAMEBUFFER_COMPLETE_EXT */: return "GL_FRAMEBUFFER_COMPLETE_EXT
    // (0x8CD5)"; case 0x8CD6 /* GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT */:
    // return "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT (0x8CD6)"; case 0x8CD7 /*
    // GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT */: return
    // "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT (0x8CD7)";
    case 0x8CD9 /* GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT */:
      return "GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT (0x8CD9)";
    case 0x8CDA /* GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT */:
      return "GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT (0x8CDA)";
    // case 0x8CDB /* GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT */: return
    // "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT (0x8CDB)"; case 0x8CDC /*
    // GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT */: return
    // "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT (0x8CDC)"; case 0x8CDD /*
    // GL_FRAMEBUFFER_UNSUPPORTED_EXT */: return "GL_FRAMEBUFFER_UNSUPPORTED_EXT
    // (0x8CDD)"; case 0x8CDF /* GL_MAX_COLOR_ATTACHMENTS_EXT */: return
    // "GL_MAX_COLOR_ATTACHMENTS_EXT (0x8CDF)"; case 0x8CE0 /*
    // GL_COLOR_ATTACHMENT0_EXT */: return "GL_COLOR_ATTACHMENT0_EXT (0x8CE0)";
    // case 0x8CE1 /* GL_COLOR_ATTACHMENT1_EXT */: return
    // "GL_COLOR_ATTACHMENT1_EXT (0x8CE1)"; case 0x8CE2 /*
    // GL_COLOR_ATTACHMENT2_EXT */: return "GL_COLOR_ATTACHMENT2_EXT (0x8CE2)";
    // case 0x8CE3 /* GL_COLOR_ATTACHMENT3_EXT */: return
    // "GL_COLOR_ATTACHMENT3_EXT (0x8CE3)"; case 0x8CE4 /*
    // GL_COLOR_ATTACHMENT4_EXT */: return "GL_COLOR_ATTACHMENT4_EXT (0x8CE4)";
    // case 0x8CE5 /* GL_COLOR_ATTACHMENT5_EXT */: return
    // "GL_COLOR_ATTACHMENT5_EXT (0x8CE5)"; case 0x8CE6 /*
    // GL_COLOR_ATTACHMENT6_EXT */: return "GL_COLOR_ATTACHMENT6_EXT (0x8CE6)";
    // case 0x8CE7 /* GL_COLOR_ATTACHMENT7_EXT */: return
    // "GL_COLOR_ATTACHMENT7_EXT (0x8CE7)"; case 0x8CE8 /*
    // GL_COLOR_ATTACHMENT8_EXT */: return "GL_COLOR_ATTACHMENT8_EXT (0x8CE8)";
    // case 0x8CE9 /* GL_COLOR_ATTACHMENT9_EXT */: return
    // "GL_COLOR_ATTACHMENT9_EXT (0x8CE9)"; case 0x8CEA /*
    // GL_COLOR_ATTACHMENT10_EXT */: return "GL_COLOR_ATTACHMENT10_EXT (0x8CEA)";
    // case 0x8CEB /* GL_COLOR_ATTACHMENT11_EXT */: return
    // "GL_COLOR_ATTACHMENT11_EXT (0x8CEB)"; case 0x8CEC /*
    // GL_COLOR_ATTACHMENT12_EXT */: return "GL_COLOR_ATTACHMENT12_EXT (0x8CEC)";
    // case 0x8CED /* GL_COLOR_ATTACHMENT13_EXT */: return
    // "GL_COLOR_ATTACHMENT13_EXT (0x8CED)"; case 0x8CEE /*
    // GL_COLOR_ATTACHMENT14_EXT */: return "GL_COLOR_ATTACHMENT14_EXT (0x8CEE)";
    // case 0x8CEF /* GL_COLOR_ATTACHMENT15_EXT */: return
    // "GL_COLOR_ATTACHMENT15_EXT (0x8CEF)"; case 0x8D00 /*
    // GL_DEPTH_ATTACHMENT_EXT */: return "GL_DEPTH_ATTACHMENT_EXT (0x8D00)";
    // case 0x8D20 /* GL_STENCIL_ATTACHMENT_EXT */: return
    // "GL_STENCIL_ATTACHMENT_EXT (0x8D20)"; case 0x8D40 /* GL_FRAMEBUFFER_EXT
    // */: return "GL_FRAMEBUFFER_EXT (0x8D40)"; case 0x8D41 /*
    // GL_RENDERBUFFER_EXT */: return "GL_RENDERBUFFER_EXT (0x8D41)"; case 0x8D42
    // /* GL_RENDERBUFFER_WIDTH_EXT */: return "GL_RENDERBUFFER_WIDTH_EXT
    // (0x8D42)"; case 0x8D43 /* GL_RENDERBUFFER_HEIGHT_EXT */: return
    // "GL_RENDERBUFFER_HEIGHT_EXT (0x8D43)"; case 0x8D44 /*
    // GL_RENDERBUFFER_INTERNAL_FORMAT_EXT */: return
    // "GL_RENDERBUFFER_INTERNAL_FORMAT_EXT (0x8D44)"; case 0x8D46 /*
    // GL_STENCIL_INDEX1_EXT */: return "GL_STENCIL_INDEX1_EXT (0x8D46)"; case
    // 0x8D47 /* GL_STENCIL_INDEX4_EXT */: return "GL_STENCIL_INDEX4_EXT
    // (0x8D47)"; case 0x8D48 /* GL_STENCIL_INDEX8_EXT */: return
    // "GL_STENCIL_INDEX8_EXT (0x8D48)"; case 0x8D49 /* GL_STENCIL_INDEX16_EXT
    // */: return "GL_STENCIL_INDEX16_EXT (0x8D49)"; case 0x8D50 /*
    // GL_RENDERBUFFER_RED_SIZE_EXT */: return "GL_RENDERBUFFER_RED_SIZE_EXT
    // (0x8D50)"; case 0x8D51 /* GL_RENDERBUFFER_GREEN_SIZE_EXT */: return
    // "GL_RENDERBUFFER_GREEN_SIZE_EXT (0x8D51)"; case 0x8D52 /*
    // GL_RENDERBUFFER_BLUE_SIZE_EXT */: return "GL_RENDERBUFFER_BLUE_SIZE_EXT
    // (0x8D52)"; case 0x8D53 /* GL_RENDERBUFFER_ALPHA_SIZE_EXT */: return
    // "GL_RENDERBUFFER_ALPHA_SIZE_EXT (0x8D53)"; case 0x8D54 /*
    // GL_RENDERBUFFER_DEPTH_SIZE_EXT */: return "GL_RENDERBUFFER_DEPTH_SIZE_EXT
    // (0x8D54)"; case 0x8D55 /* GL_RENDERBUFFER_STENCIL_SIZE_EXT */: return
    // "GL_RENDERBUFFER_STENCIL_SIZE_EXT (0x8D55)";

    /* ------------------------ GL_EXT_framebuffer_sRGB ------------------------
     */

    // case 0x8DB9 /* GL_FRAMEBUFFER_SRGB_EXT */: return
    // "GL_FRAMEBUFFER_SRGB_EXT (0x8DB9)";
    case 0x8DBA /* GL_FRAMEBUFFER_SRGB_CAPABLE_EXT */:
      return "GL_FRAMEBUFFER_SRGB_CAPABLE_EXT (0x8DBA)";

      /* ------------------------ GL_EXT_geometry_shader4
       * ------------------------ */

      // case 0xA /* GL_LINES_ADJACENCY_EXT */: return "GL_LINES_ADJACENCY_EXT
      // (0xA)"; case 0xB /* GL_LINE_STRIP_ADJACENCY_EXT */: return
      // "GL_LINE_STRIP_ADJACENCY_EXT (0xB)"; case 0xC /*
      // GL_TRIANGLES_ADJACENCY_EXT */: return "GL_TRIANGLES_ADJACENCY_EXT
      // (0xC)"; case 0xD /* GL_TRIANGLE_STRIP_ADJACENCY_EXT */: return
      // "GL_TRIANGLE_STRIP_ADJACENCY_EXT (0xD)"; case 0x8642 /*
      // GL_PROGRAM_POINT_SIZE_EXT */: return "GL_PROGRAM_POINT_SIZE_EXT
      // (0x8642)"; case 0x8B4B /* GL_MAX_VARYING_COMPONENTS_EXT */: return
      // "GL_MAX_VARYING_COMPONENTS_EXT (0x8B4B)"; case 0x8C29 /*
      // GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS_EXT */: return
      // "GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS_EXT (0x8C29)"; case 0x8CD4 /*
      // GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER_EXT */: return
      // "GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER_EXT (0x8CD4)"; case 0x8DA7 /*
      // GL_FRAMEBUFFER_ATTACHMENT_LAYERED_EXT */: return
      // "GL_FRAMEBUFFER_ATTACHMENT_LAYERED_EXT (0x8DA7)"; case 0x8DA8 /*
      // GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS_EXT */: return
      // "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS_EXT (0x8DA8)"; case 0x8DA9 /*
      // GL_FRAMEBUFFER_INCOMPLETE_LAYER_COUNT_EXT */: return
      // "GL_FRAMEBUFFER_INCOMPLETE_LAYER_COUNT_EXT (0x8DA9)"; case 0x8DD9 /*
      // GL_GEOMETRY_SHADER_EXT */: return "GL_GEOMETRY_SHADER_EXT (0x8DD9)";
      // case 0x8DDA /* GL_GEOMETRY_VERTICES_OUT_EXT */: return
      // "GL_GEOMETRY_VERTICES_OUT_EXT (0x8DDA)"; case 0x8DDB /*
      // GL_GEOMETRY_INPUT_TYPE_EXT */: return "GL_GEOMETRY_INPUT_TYPE_EXT
      // (0x8DDB)"; case 0x8DDC /* GL_GEOMETRY_OUTPUT_TYPE_EXT */: return
      // "GL_GEOMETRY_OUTPUT_TYPE_EXT (0x8DDC)"; case 0x8DDD /*
      // GL_MAX_GEOMETRY_VARYING_COMPONENTS_EXT */: return
      // "GL_MAX_GEOMETRY_VARYING_COMPONENTS_EXT (0x8DDD)"; case 0x8DDE /*
      // GL_MAX_VERTEX_VARYING_COMPONENTS_EXT */: return
      // "GL_MAX_VERTEX_VARYING_COMPONENTS_EXT (0x8DDE)"; case 0x8DDF /*
      // GL_MAX_GEOMETRY_UNIFORM_COMPONENTS_EXT */: return
      // "GL_MAX_GEOMETRY_UNIFORM_COMPONENTS_EXT (0x8DDF)"; case 0x8DE0 /*
      // GL_MAX_GEOMETRY_OUTPUT_VERTICES_EXT */: return
      // "GL_MAX_GEOMETRY_OUTPUT_VERTICES_EXT (0x8DE0)"; case 0x8DE1 /*
      // GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS_EXT */: return
      // "GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS_EXT (0x8DE1)";

      /* --------------------- GL_EXT_gpu_program_parameters
       * --------------------- */

      /* --------------------------- GL_EXT_gpu_shader4
       * -------------------------- */

      // case 0x88FD /* GL_VERTEX_ATTRIB_ARRAY_INTEGER_EXT */: return
      // "GL_VERTEX_ATTRIB_ARRAY_INTEGER_EXT (0x88FD)"; case 0x8DC0 /*
      // GL_SAMPLER_1D_ARRAY_EXT */: return "GL_SAMPLER_1D_ARRAY_EXT (0x8DC0)";
      // case 0x8DC1 /* GL_SAMPLER_2D_ARRAY_EXT */: return
      // "GL_SAMPLER_2D_ARRAY_EXT (0x8DC1)"; case 0x8DC2 /* GL_SAMPLER_BUFFER_EXT
      // */: return "GL_SAMPLER_BUFFER_EXT (0x8DC2)"; case 0x8DC3 /*
      // GL_SAMPLER_1D_ARRAY_SHADOW_EXT */: return
      // "GL_SAMPLER_1D_ARRAY_SHADOW_EXT (0x8DC3)"; case 0x8DC4 /*
      // GL_SAMPLER_2D_ARRAY_SHADOW_EXT */: return
      // "GL_SAMPLER_2D_ARRAY_SHADOW_EXT (0x8DC4)"; case 0x8DC5 /*
      // GL_SAMPLER_CUBE_SHADOW_EXT */: return "GL_SAMPLER_CUBE_SHADOW_EXT
      // (0x8DC5)"; case 0x8DC6 /* GL_UNSIGNED_INT_VEC2_EXT */: return
      // "GL_UNSIGNED_INT_VEC2_EXT (0x8DC6)"; case 0x8DC7 /*
      // GL_UNSIGNED_INT_VEC3_EXT */: return "GL_UNSIGNED_INT_VEC3_EXT (0x8DC7)";
      // case 0x8DC8 /* GL_UNSIGNED_INT_VEC4_EXT */: return
      // "GL_UNSIGNED_INT_VEC4_EXT (0x8DC8)"; case 0x8DC9 /*
      // GL_INT_SAMPLER_1D_EXT */: return "GL_INT_SAMPLER_1D_EXT (0x8DC9)"; case
      // 0x8DCA /* GL_INT_SAMPLER_2D_EXT */: return "GL_INT_SAMPLER_2D_EXT
      // (0x8DCA)"; case 0x8DCB /* GL_INT_SAMPLER_3D_EXT */: return
      // "GL_INT_SAMPLER_3D_EXT (0x8DCB)"; case 0x8DCC /* GL_INT_SAMPLER_CUBE_EXT
      // */: return "GL_INT_SAMPLER_CUBE_EXT (0x8DCC)"; case 0x8DCD /*
      // GL_INT_SAMPLER_2D_RECT_EXT */: return "GL_INT_SAMPLER_2D_RECT_EXT
      // (0x8DCD)"; case 0x8DCE /* GL_INT_SAMPLER_1D_ARRAY_EXT */: return
      // "GL_INT_SAMPLER_1D_ARRAY_EXT (0x8DCE)"; case 0x8DCF /*
      // GL_INT_SAMPLER_2D_ARRAY_EXT */: return "GL_INT_SAMPLER_2D_ARRAY_EXT
      // (0x8DCF)"; case 0x8DD0 /* GL_INT_SAMPLER_BUFFER_EXT */: return
      // "GL_INT_SAMPLER_BUFFER_EXT (0x8DD0)"; case 0x8DD1 /*
      // GL_UNSIGNED_INT_SAMPLER_1D_EXT */: return
      // "GL_UNSIGNED_INT_SAMPLER_1D_EXT (0x8DD1)"; case 0x8DD2 /*
      // GL_UNSIGNED_INT_SAMPLER_2D_EXT */: return
      // "GL_UNSIGNED_INT_SAMPLER_2D_EXT (0x8DD2)"; case 0x8DD3 /*
      // GL_UNSIGNED_INT_SAMPLER_3D_EXT */: return
      // "GL_UNSIGNED_INT_SAMPLER_3D_EXT (0x8DD3)"; case 0x8DD4 /*
      // GL_UNSIGNED_INT_SAMPLER_CUBE_EXT */: return
      // "GL_UNSIGNED_INT_SAMPLER_CUBE_EXT (0x8DD4)"; case 0x8DD5 /*
      // GL_UNSIGNED_INT_SAMPLER_2D_RECT_EXT */: return
      // "GL_UNSIGNED_INT_SAMPLER_2D_RECT_EXT (0x8DD5)"; case 0x8DD6 /*
      // GL_UNSIGNED_INT_SAMPLER_1D_ARRAY_EXT */: return
      // "GL_UNSIGNED_INT_SAMPLER_1D_ARRAY_EXT (0x8DD6)"; case 0x8DD7 /*
      // GL_UNSIGNED_INT_SAMPLER_2D_ARRAY_EXT */: return
      // "GL_UNSIGNED_INT_SAMPLER_2D_ARRAY_EXT (0x8DD7)"; case 0x8DD8 /*
      // GL_UNSIGNED_INT_SAMPLER_BUFFER_EXT */: return
      // "GL_UNSIGNED_INT_SAMPLER_BUFFER_EXT (0x8DD8)";

      /* ---------------------------- GL_EXT_histogram
       * --------------------------- */

      // case 0x8024 /* GL_HISTOGRAM_EXT */: return "GL_HISTOGRAM_EXT (0x8024)";
      // case 0x8025 /* GL_PROXY_HISTOGRAM_EXT */: return
      // "GL_PROXY_HISTOGRAM_EXT (0x8025)"; case 0x8026 /* GL_HISTOGRAM_WIDTH_EXT
      // */: return "GL_HISTOGRAM_WIDTH_EXT (0x8026)"; case 0x8027 /*
      // GL_HISTOGRAM_FORMAT_EXT */: return "GL_HISTOGRAM_FORMAT_EXT (0x8027)";
      // case 0x8028 /* GL_HISTOGRAM_RED_SIZE_EXT */: return
      // "GL_HISTOGRAM_RED_SIZE_EXT (0x8028)"; case 0x8029 /*
      // GL_HISTOGRAM_GREEN_SIZE_EXT */: return "GL_HISTOGRAM_GREEN_SIZE_EXT
      // (0x8029)"; case 0x802A /* GL_HISTOGRAM_BLUE_SIZE_EXT */: return
      // "GL_HISTOGRAM_BLUE_SIZE_EXT (0x802A)"; case 0x802B /*
      // GL_HISTOGRAM_ALPHA_SIZE_EXT */: return "GL_HISTOGRAM_ALPHA_SIZE_EXT
      // (0x802B)"; case 0x802C /* GL_HISTOGRAM_LUMINANCE_SIZE_EXT */: return
      // "GL_HISTOGRAM_LUMINANCE_SIZE_EXT (0x802C)"; case 0x802D /*
      // GL_HISTOGRAM_SINK_EXT */: return "GL_HISTOGRAM_SINK_EXT (0x802D)"; case
      // 0x802E /* GL_MINMAX_EXT */: return "GL_MINMAX_EXT (0x802E)"; case 0x802F
      // /* GL_MINMAX_FORMAT_EXT */: return "GL_MINMAX_FORMAT_EXT (0x802F)"; case
      // 0x8030 /* GL_MINMAX_SINK_EXT */: return "GL_MINMAX_SINK_EXT (0x8030)";

      /* ----------------------- GL_EXT_index_array_formats
       * ---------------------- */

      /* --------------------------- GL_EXT_index_func
       * --------------------------- */

      /* ------------------------- GL_EXT_index_material
       * ------------------------- */

      /* -------------------------- GL_EXT_index_texture
       * ------------------------- */

      /* -------------------------- GL_EXT_light_texture
       * ------------------------- */

    case 0x8349 /* GL_FRAGMENT_MATERIAL_EXT */:
      return "GL_FRAGMENT_MATERIAL_EXT (0x8349)";
    case 0x834A /* GL_FRAGMENT_NORMAL_EXT */:
      return "GL_FRAGMENT_NORMAL_EXT (0x834A)";
    case 0x834C /* GL_FRAGMENT_COLOR_EXT */:
      return "GL_FRAGMENT_COLOR_EXT (0x834C)";
    case 0x834D /* GL_ATTENUATION_EXT */:
      return "GL_ATTENUATION_EXT (0x834D)";
    case 0x834E /* GL_SHADOW_ATTENUATION_EXT */:
      return "GL_SHADOW_ATTENUATION_EXT (0x834E)";
    case 0x834F /* GL_TEXTURE_APPLICATION_MODE_EXT */:
      return "GL_TEXTURE_APPLICATION_MODE_EXT (0x834F)";
    case 0x8350 /* GL_TEXTURE_LIGHT_EXT */:
      return "GL_TEXTURE_LIGHT_EXT (0x8350)";
    case 0x8351 /* GL_TEXTURE_MATERIAL_FACE_EXT */:
      return "GL_TEXTURE_MATERIAL_FACE_EXT (0x8351)";
    case 0x8352 /* GL_TEXTURE_MATERIAL_PARAMETER_EXT */:
      return "GL_TEXTURE_MATERIAL_PARAMETER_EXT (0x8352)";

    /* ------------------------- GL_EXT_misc_attribute -------------------------
     */

    /* ------------------------ GL_EXT_multi_draw_arrays -----------------------
     */

    /* --------------------------- GL_EXT_multisample --------------------------
     */

    // case 0x809D /* GL_MULTISAMPLE_EXT */: return "GL_MULTISAMPLE_EXT
    // (0x809D)"; case 0x809E /* GL_SAMPLE_ALPHA_TO_MASK_EXT */: return
    // "GL_SAMPLE_ALPHA_TO_MASK_EXT (0x809E)"; case 0x809F /*
    // GL_SAMPLE_ALPHA_TO_ONE_EXT */: return "GL_SAMPLE_ALPHA_TO_ONE_EXT
    // (0x809F)"; case 0x80A0 /* GL_SAMPLE_MASK_EXT */: return
    // "GL_SAMPLE_MASK_EXT (0x80A0)";
    case 0x80A1 /* GL_1PASS_EXT */:
      return "GL_1PASS_EXT (0x80A1)";
    case 0x80A2 /* GL_2PASS_0_EXT */:
      return "GL_2PASS_0_EXT (0x80A2)";
    case 0x80A3 /* GL_2PASS_1_EXT */:
      return "GL_2PASS_1_EXT (0x80A3)";
    case 0x80A4 /* GL_4PASS_0_EXT */:
      return "GL_4PASS_0_EXT (0x80A4)";
    case 0x80A5 /* GL_4PASS_1_EXT */:
      return "GL_4PASS_1_EXT (0x80A5)";
    case 0x80A6 /* GL_4PASS_2_EXT */:
      return "GL_4PASS_2_EXT (0x80A6)";
    case 0x80A7 /* GL_4PASS_3_EXT */:
      return "GL_4PASS_3_EXT (0x80A7)";
    // case 0x80A8 /* GL_SAMPLE_BUFFERS_EXT */: return "GL_SAMPLE_BUFFERS_EXT
    // (0x80A8)"; case 0x80A9 /* GL_SAMPLES_EXT */: return "GL_SAMPLES_EXT
    // (0x80A9)"; case 0x80AA /* GL_SAMPLE_MASK_VALUE_EXT */: return
    // "GL_SAMPLE_MASK_VALUE_EXT (0x80AA)"; case 0x80AB /*
    // GL_SAMPLE_MASK_INVERT_EXT */: return "GL_SAMPLE_MASK_INVERT_EXT (0x80AB)";
    case 0x80AC /* GL_SAMPLE_PATTERN_EXT */:
      return "GL_SAMPLE_PATTERN_EXT (0x80AC)";
    // case 0x20000000 /* GL_MULTISAMPLE_BIT_EXT */: return
    // "GL_MULTISAMPLE_BIT_EXT (0x20000000)";

    /* ---------------------- GL_EXT_packed_depth_stencil ----------------------
     */

    // case 0x84F9 /* GL_DEPTH_STENCIL_EXT */: return "GL_DEPTH_STENCIL_EXT
    // (0x84F9)"; case 0x84FA /* GL_UNSIGNED_INT_24_8_EXT */: return
    // "GL_UNSIGNED_INT_24_8_EXT (0x84FA)"; case 0x88F0 /*
    // GL_DEPTH24_STENCIL8_EXT */: return "GL_DEPTH24_STENCIL8_EXT (0x88F0)";
    // case 0x88F1 /* GL_TEXTURE_STENCIL_SIZE_EXT */: return
    // "GL_TEXTURE_STENCIL_SIZE_EXT (0x88F1)";

    /* -------------------------- GL_EXT_packed_float --------------------------
     */

    // case 0x8C3A /* GL_R11F_G11F_B10F_EXT */: return "GL_R11F_G11F_B10F_EXT
    // (0x8C3A)"; case 0x8C3B /* GL_UNSIGNED_INT_10F_11F_11F_REV_EXT */: return
    // "GL_UNSIGNED_INT_10F_11F_11F_REV_EXT (0x8C3B)";
    case 0x8C3C /* GL_RGBA_SIGNED_COMPONENTS_EXT */:
      return "GL_RGBA_SIGNED_COMPONENTS_EXT (0x8C3C)";

    /* -------------------------- GL_EXT_packed_pixels -------------------------
     */

    /// case 0x8032 /* GL_UNSIGNED_BYTE_3_3_2_EXT */: return
    /// "GL_UNSIGNED_BYTE_3_3_2_EXT (0x8032)";
    // case 0x8033 /* GL_UNSIGNED_SHORT_4_4_4_4_EXT */: return
    // "GL_UNSIGNED_SHORT_4_4_4_4_EXT (0x8033)"; case 0x8034 /*
    // GL_UNSIGNED_SHORT_5_5_5_1_EXT */: return "GL_UNSIGNED_SHORT_5_5_5_1_EXT
    // (0x8034)"; case 0x8035 /* GL_UNSIGNED_INT_8_8_8_8_EXT */: return
    // "GL_UNSIGNED_INT_8_8_8_8_EXT (0x8035)"; case 0x8036 /*
    // GL_UNSIGNED_INT_10_10_10_2_EXT */: return "GL_UNSIGNED_INT_10_10_10_2_EXT
    // (0x8036)";

    /* ------------------------ GL_EXT_paletted_texture ------------------------
     */

    // case 0x0DE0 /* GL_TEXTURE_1D */: return "GL_TEXTURE_1D (0x0DE0)";
    // case 0x0DE1 /* GL_TEXTURE_2D */: return "GL_TEXTURE_2D (0x0DE1)";
    // case 0x8063 /* GL_PROXY_TEXTURE_1D */: return "GL_PROXY_TEXTURE_1D
    // (0x8063)"; case 0x8064 /* GL_PROXY_TEXTURE_2D */: return
    // "GL_PROXY_TEXTURE_2D (0x8064)"; case 0x80D8 /* GL_COLOR_TABLE_FORMAT_EXT
    // */: return "GL_COLOR_TABLE_FORMAT_EXT (0x80D8)"; case 0x80D9 /*
    // GL_COLOR_TABLE_WIDTH_EXT */: return "GL_COLOR_TABLE_WIDTH_EXT (0x80D9)";
    // case 0x80DA /* GL_COLOR_TABLE_RED_SIZE_EXT */: return
    // "GL_COLOR_TABLE_RED_SIZE_EXT (0x80DA)"; case 0x80DB /*
    // GL_COLOR_TABLE_GREEN_SIZE_EXT */: return "GL_COLOR_TABLE_GREEN_SIZE_EXT
    // (0x80DB)"; case 0x80DC /* GL_COLOR_TABLE_BLUE_SIZE_EXT */: return
    // "GL_COLOR_TABLE_BLUE_SIZE_EXT (0x80DC)"; case 0x80DD /*
    // GL_COLOR_TABLE_ALPHA_SIZE_EXT */: return "GL_COLOR_TABLE_ALPHA_SIZE_EXT
    // (0x80DD)"; case 0x80DE /* GL_COLOR_TABLE_LUMINANCE_SIZE_EXT */: return
    // "GL_COLOR_TABLE_LUMINANCE_SIZE_EXT (0x80DE)"; case 0x80DF /*
    // GL_COLOR_TABLE_INTENSITY_SIZE_EXT */: return
    // "GL_COLOR_TABLE_INTENSITY_SIZE_EXT (0x80DF)"; case 0x80E2 /*
    // GL_COLOR_INDEX1_EXT */: return "GL_COLOR_INDEX1_EXT (0x80E2)"; case 0x80E3
    // /* GL_COLOR_INDEX2_EXT */: return "GL_COLOR_INDEX2_EXT (0x80E3)"; case
    // 0x80E4 /* GL_COLOR_INDEX4_EXT */: return "GL_COLOR_INDEX4_EXT (0x80E4)";
    // case 0x80E5 /* GL_COLOR_INDEX8_EXT */: return "GL_COLOR_INDEX8_EXT
    // (0x80E5)"; case 0x80E6 /* GL_COLOR_INDEX12_EXT */: return
    // "GL_COLOR_INDEX12_EXT (0x80E6)"; case 0x80E7 /* GL_COLOR_INDEX16_EXT */:
    // return "GL_COLOR_INDEX16_EXT (0x80E7)";
    case 0x80ED /* GL_TEXTURE_INDEX_SIZE_EXT */:
      return "GL_TEXTURE_INDEX_SIZE_EXT (0x80ED)";
      // case 0x8513 /* GL_TEXTURE_CUBE_MAP_ARB */: return
      // "GL_TEXTURE_CUBE_MAP_ARB (0x8513)"; case 0x851B /*
      // GL_PROXY_TEXTURE_CUBE_MAP_ARB */: return "GL_PROXY_TEXTURE_CUBE_MAP_ARB
      // (0x851B)";

      /* ----------------------- GL_EXT_pixel_buffer_object
       * ---------------------- */

      // case 0x88EB /* GL_PIXEL_PACK_BUFFER_EXT */: return
      // "GL_PIXEL_PACK_BUFFER_EXT (0x88EB)"; case 0x88EC /*
      // GL_PIXEL_UNPACK_BUFFER_EXT */: return "GL_PIXEL_UNPACK_BUFFER_EXT
      // (0x88EC)"; case 0x88ED /* GL_PIXEL_PACK_BUFFER_BINDING_EXT */: return
      // "GL_PIXEL_PACK_BUFFER_BINDING_EXT (0x88ED)"; case 0x88EF /*
      // GL_PIXEL_UNPACK_BUFFER_BINDING_EXT */: return
      // "GL_PIXEL_UNPACK_BUFFER_BINDING_EXT (0x88EF)";

      /* ------------------------- GL_EXT_pixel_transform
       * ------------------------ */

    case 0x8330 /* GL_PIXEL_TRANSFORM_2D_EXT */:
      return "GL_PIXEL_TRANSFORM_2D_EXT (0x8330)";
    case 0x8331 /* GL_PIXEL_MAG_FILTER_EXT */:
      return "GL_PIXEL_MAG_FILTER_EXT (0x8331)";
    case 0x8332 /* GL_PIXEL_MIN_FILTER_EXT */:
      return "GL_PIXEL_MIN_FILTER_EXT (0x8332)";
    case 0x8333 /* GL_PIXEL_CUBIC_WEIGHT_EXT */:
      return "GL_PIXEL_CUBIC_WEIGHT_EXT (0x8333)";
    case 0x8334 /* GL_CUBIC_EXT */:
      return "GL_CUBIC_EXT (0x8334)";
    case 0x8335 /* GL_AVERAGE_EXT */:
      return "GL_AVERAGE_EXT (0x8335)";
    case 0x8336 /* GL_PIXEL_TRANSFORM_2D_STACK_DEPTH_EXT */:
      return "GL_PIXEL_TRANSFORM_2D_STACK_DEPTH_EXT (0x8336)";
    case 0x8337 /* GL_MAX_PIXEL_TRANSFORM_2D_STACK_DEPTH_EXT */:
      return "GL_MAX_PIXEL_TRANSFORM_2D_STACK_DEPTH_EXT (0x8337)";
    case 0x8338 /* GL_PIXEL_TRANSFORM_2D_MATRIX_EXT */:
      return "GL_PIXEL_TRANSFORM_2D_MATRIX_EXT (0x8338)"; /* -------------------
                                                             GL_EXT_pixel_transform_color_table
                                                             ------------------
                                                           */

    /* ------------------------ GL_EXT_point_parameters ------------------------
     */

    // case 0x8126 /* GL_POINT_SIZE_MIN_EXT */: return "GL_POINT_SIZE_MIN_EXT
    // (0x8126)"; case 0x8127 /* GL_POINT_SIZE_MAX_EXT */: return
    // "GL_POINT_SIZE_MAX_EXT (0x8127)"; case 0x8128 /*
    // GL_POINT_FADE_THRESHOLD_SIZE_EXT */: return
    // "GL_POINT_FADE_THRESHOLD_SIZE_EXT (0x8128)"; case 0x8129 /*
    // GL_DISTANCE_ATTENUATION_EXT */: return "GL_DISTANCE_ATTENUATION_EXT
    // (0x8129)";

    /* ------------------------- GL_EXT_polygon_offset -------------------------
     */

    // case 0x8037 /* GL_POLYGON_OFFSET_EXT */: return "GL_POLYGON_OFFSET_EXT
    // (0x8037)"; case 0x8038 /* GL_POLYGON_OFFSET_FACTOR_EXT */: return
    // "GL_POLYGON_OFFSET_FACTOR_EXT (0x8038)";
    case 0x8039 /* GL_POLYGON_OFFSET_BIAS_EXT */:
      return "GL_POLYGON_OFFSET_BIAS_EXT (0x8039)"; /* ------------------------
                                                       GL_EXT_provoking_vertex
                                                       ------------------------
                                                     */

      // case 0x8E4C /* GL_QUADS_FOLLOW_PROVOKING_VERTEX_CONVENTION_EXT */:
      // return "GL_QUADS_FOLLOW_PROVOKING_VERTEX_CONVENTION_EXT (0x8E4C)"; case
      // 0x8E4D /* GL_FIRST_VERTEX_CONVENTION_EXT */: return
      // "GL_FIRST_VERTEX_CONVENTION_EXT (0x8E4D)"; case 0x8E4E /*
      // GL_LAST_VERTEX_CONVENTION_EXT */: return "GL_LAST_VERTEX_CONVENTION_EXT
      // (0x8E4E)"; case 0x8E4F /* GL_PROVOKING_VERTEX_EXT */: return
      // "GL_PROVOKING_VERTEX_EXT (0x8E4F)";/* -------------------------
      // GL_EXT_rescale_normal ------------------------- */

      // case 0x803A /* GL_RESCALE_NORMAL_EXT */: return "GL_RESCALE_NORMAL_EXT
      // (0x803A)";

      /* -------------------------- GL_EXT_scene_marker
       * -------------------------- */

      /* ------------------------- GL_EXT_secondary_color
       * ------------------------ */

      // case 0x8458 /* GL_COLOR_SUM_EXT */: return "GL_COLOR_SUM_EXT (0x8458)";
      // case 0x8459 /* GL_CURRENT_SECONDARY_COLOR_EXT */: return
      // "GL_CURRENT_SECONDARY_COLOR_EXT (0x8459)"; case 0x845A /*
      // GL_SECONDARY_COLOR_ARRAY_SIZE_EXT */: return
      // "GL_SECONDARY_COLOR_ARRAY_SIZE_EXT (0x845A)"; case 0x845B /*
      // GL_SECONDARY_COLOR_ARRAY_TYPE_EXT */: return
      // "GL_SECONDARY_COLOR_ARRAY_TYPE_EXT (0x845B)"; case 0x845C /*
      // GL_SECONDARY_COLOR_ARRAY_STRIDE_EXT */: return
      // "GL_SECONDARY_COLOR_ARRAY_STRIDE_EXT (0x845C)"; case 0x845D /*
      // GL_SECONDARY_COLOR_ARRAY_POINTER_EXT */: return
      // "GL_SECONDARY_COLOR_ARRAY_POINTER_EXT (0x845D)"; case 0x845E /*
      // GL_SECONDARY_COLOR_ARRAY_EXT */: return "GL_SECONDARY_COLOR_ARRAY_EXT
      // (0x845E)";

      /* --------------------- GL_EXT_separate_shader_objects
       * -------------------- */

      // case 0x8B8D /* GL_ACTIVE_PROGRAM_EXT */: return "GL_ACTIVE_PROGRAM_EXT
      // (0x8B8D)";

      /* --------------------- GL_EXT_separate_specular_color
       * -------------------- */

      // case 0x81F8 /* GL_LIGHT_MODEL_COLOR_CONTROL_EXT */: return
      // "GL_LIGHT_MODEL_COLOR_CONTROL_EXT (0x81F8)"; case 0x81F9 /*
      // GL_SINGLE_COLOR_EXT */: return "GL_SINGLE_COLOR_EXT (0x81F9)"; case
      // 0x81FA /* GL_SEPARATE_SPECULAR_COLOR_EXT */: return
      // "GL_SEPARATE_SPECULAR_COLOR_EXT (0x81FA)";

      /* --------------------- GL_EXT_shader_image_load_store
       * -------------------- */

      // case 0x00000001 /* GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT_EXT */: return
      // "GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT_EXT (0x00000001)"; case 0x00000002
      // /* GL_ELEMENT_ARRAY_BARRIER_BIT_EXT */: return
      // "GL_ELEMENT_ARRAY_BARRIER_BIT_EXT (0x00000002)"; case 0x00000004 /*
      // GL_UNIFORM_BARRIER_BIT_EXT */: return "GL_UNIFORM_BARRIER_BIT_EXT
      // (0x00000004)"; case 0x00000008 /* GL_TEXTURE_FETCH_BARRIER_BIT_EXT */:
      // return "GL_TEXTURE_FETCH_BARRIER_BIT_EXT (0x00000008)"; case 0x00000020
      // /* GL_SHADER_IMAGE_ACCESS_BARRIER_BIT_EXT */: return
      // "GL_SHADER_IMAGE_ACCESS_BARRIER_BIT_EXT (0x00000020)"; case 0x00000040
      // /* GL_COMMAND_BARRIER_BIT_EXT */: return "GL_COMMAND_BARRIER_BIT_EXT
      // (0x00000040)"; case 0x00000080 /* GL_PIXEL_BUFFER_BARRIER_BIT_EXT */:
      // return "GL_PIXEL_BUFFER_BARRIER_BIT_EXT (0x00000080)"; case 0x00000100
      // /* GL_TEXTURE_UPDATE_BARRIER_BIT_EXT */: return
      // "GL_TEXTURE_UPDATE_BARRIER_BIT_EXT (0x00000100)"; case 0x00000200 /*
      // GL_BUFFER_UPDATE_BARRIER_BIT_EXT */: return
      // "GL_BUFFER_UPDATE_BARRIER_BIT_EXT (0x00000200)"; case 0x00000400 /*
      // GL_FRAMEBUFFER_BARRIER_BIT_EXT */: return
      // "GL_FRAMEBUFFER_BARRIER_BIT_EXT (0x00000400)"; case 0x00000800 /*
      // GL_TRANSFORM_FEEDBACK_BARRIER_BIT_EXT */: return
      // "GL_TRANSFORM_FEEDBACK_BARRIER_BIT_EXT (0x00000800)"; case 0x00001000 /*
      // GL_ATOMIC_COUNTER_BARRIER_BIT_EXT */: return
      // "GL_ATOMIC_COUNTER_BARRIER_BIT_EXT (0x00001000)"; case 0x8F38 /*
      // GL_MAX_IMAGE_UNITS_EXT */: return "GL_MAX_IMAGE_UNITS_EXT (0x8F38)";
      // case 0x8F39 /* GL_MAX_COMBINED_IMAGE_UNITS_AND_FRAGMENT_OUTPUTS_EXT */:
      // return "GL_MAX_COMBINED_IMAGE_UNITS_AND_FRAGMENT_OUTPUTS_EXT (0x8F39)";
      // case 0x8F3A /* GL_IMAGE_BINDING_NAME_EXT */: return
      // "GL_IMAGE_BINDING_NAME_EXT (0x8F3A)"; case 0x8F3B /*
      // GL_IMAGE_BINDING_LEVEL_EXT */: return "GL_IMAGE_BINDING_LEVEL_EXT
      // (0x8F3B)"; case 0x8F3C /* GL_IMAGE_BINDING_LAYERED_EXT */: return
      // "GL_IMAGE_BINDING_LAYERED_EXT (0x8F3C)"; case 0x8F3D /*
      // GL_IMAGE_BINDING_LAYER_EXT */: return "GL_IMAGE_BINDING_LAYER_EXT
      // (0x8F3D)"; case 0x8F3E /* GL_IMAGE_BINDING_ACCESS_EXT */: return
      // "GL_IMAGE_BINDING_ACCESS_EXT (0x8F3E)"; case 0x904C /* GL_IMAGE_1D_EXT
      // */: return "GL_IMAGE_1D_EXT (0x904C)"; case 0x904D /* GL_IMAGE_2D_EXT
      // */: return "GL_IMAGE_2D_EXT (0x904D)"; case 0x904E /* GL_IMAGE_3D_EXT
      // */: return "GL_IMAGE_3D_EXT (0x904E)"; case 0x904F /*
      // GL_IMAGE_2D_RECT_EXT */: return "GL_IMAGE_2D_RECT_EXT (0x904F)"; case
      // 0x9050 /* GL_IMAGE_CUBE_EXT */: return "GL_IMAGE_CUBE_EXT (0x9050)";
      // case 0x9051 /* GL_IMAGE_BUFFER_EXT */: return "GL_IMAGE_BUFFER_EXT
      // (0x9051)"; case 0x9052 /* GL_IMAGE_1D_ARRAY_EXT */: return
      // "GL_IMAGE_1D_ARRAY_EXT (0x9052)"; case 0x9053 /* GL_IMAGE_2D_ARRAY_EXT
      // */: return "GL_IMAGE_2D_ARRAY_EXT (0x9053)"; case 0x9054 /*
      // GL_IMAGE_CUBE_MAP_ARRAY_EXT */: return "GL_IMAGE_CUBE_MAP_ARRAY_EXT
      // (0x9054)"; case 0x9055 /* GL_IMAGE_2D_MULTISAMPLE_EXT */: return
      // "GL_IMAGE_2D_MULTISAMPLE_EXT (0x9055)"; case 0x9056 /*
      // GL_IMAGE_2D_MULTISAMPLE_ARRAY_EXT */: return
      // "GL_IMAGE_2D_MULTISAMPLE_ARRAY_EXT (0x9056)"; case 0x9057 /*
      // GL_INT_IMAGE_1D_EXT */: return "GL_INT_IMAGE_1D_EXT (0x9057)"; case
      // 0x9058 /* GL_INT_IMAGE_2D_EXT */: return "GL_INT_IMAGE_2D_EXT (0x9058)";
      // case 0x9059 /* GL_INT_IMAGE_3D_EXT */: return "GL_INT_IMAGE_3D_EXT
      // (0x9059)"; case 0x905A /* GL_INT_IMAGE_2D_RECT_EXT */: return
      // "GL_INT_IMAGE_2D_RECT_EXT (0x905A)"; case 0x905B /*
      // GL_INT_IMAGE_CUBE_EXT */: return "GL_INT_IMAGE_CUBE_EXT (0x905B)"; case
      // 0x905C /* GL_INT_IMAGE_BUFFER_EXT */: return "GL_INT_IMAGE_BUFFER_EXT
      // (0x905C)"; case 0x905D /* GL_INT_IMAGE_1D_ARRAY_EXT */: return
      // "GL_INT_IMAGE_1D_ARRAY_EXT (0x905D)"; case 0x905E /*
      // GL_INT_IMAGE_2D_ARRAY_EXT */: return "GL_INT_IMAGE_2D_ARRAY_EXT
      // (0x905E)"; case 0x905F /* GL_INT_IMAGE_CUBE_MAP_ARRAY_EXT */: return
      // "GL_INT_IMAGE_CUBE_MAP_ARRAY_EXT (0x905F)"; case 0x9060 /*
      // GL_INT_IMAGE_2D_MULTISAMPLE_EXT */: return
      // "GL_INT_IMAGE_2D_MULTISAMPLE_EXT (0x9060)"; case 0x9061 /*
      // GL_INT_IMAGE_2D_MULTISAMPLE_ARRAY_EXT */: return
      // "GL_INT_IMAGE_2D_MULTISAMPLE_ARRAY_EXT (0x9061)"; case 0x9062 /*
      // GL_UNSIGNED_INT_IMAGE_1D_EXT */: return "GL_UNSIGNED_INT_IMAGE_1D_EXT
      // (0x9062)"; case 0x9063 /* GL_UNSIGNED_INT_IMAGE_2D_EXT */: return
      // "GL_UNSIGNED_INT_IMAGE_2D_EXT (0x9063)"; case 0x9064 /*
      // GL_UNSIGNED_INT_IMAGE_3D_EXT */: return "GL_UNSIGNED_INT_IMAGE_3D_EXT
      // (0x9064)"; case 0x9065 /* GL_UNSIGNED_INT_IMAGE_2D_RECT_EXT */: return
      // "GL_UNSIGNED_INT_IMAGE_2D_RECT_EXT (0x9065)"; case 0x9066 /*
      // GL_UNSIGNED_INT_IMAGE_CUBE_EXT */: return
      // "GL_UNSIGNED_INT_IMAGE_CUBE_EXT (0x9066)"; case 0x9067 /*
      // GL_UNSIGNED_INT_IMAGE_BUFFER_EXT */: return
      // "GL_UNSIGNED_INT_IMAGE_BUFFER_EXT (0x9067)"; case 0x9068 /*
      // GL_UNSIGNED_INT_IMAGE_1D_ARRAY_EXT */: return
      // "GL_UNSIGNED_INT_IMAGE_1D_ARRAY_EXT (0x9068)"; case 0x9069 /*
      // GL_UNSIGNED_INT_IMAGE_2D_ARRAY_EXT */: return
      // "GL_UNSIGNED_INT_IMAGE_2D_ARRAY_EXT (0x9069)";
      /// case 0x906A /* GL_UNSIGNED_INT_IMAGE_CUBE_MAP_ARRAY_EXT */: return
      /// "GL_UNSIGNED_INT_IMAGE_CUBE_MAP_ARRAY_EXT (0x906A)";
      // case 0x906B /* GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_EXT */: return
      // "GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_EXT (0x906B)"; case 0x906C /*
      // GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY_EXT */: return
      // "GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY_EXT (0x906C)"; case 0x906D
      // /* GL_MAX_IMAGE_SAMPLES_EXT */: return "GL_MAX_IMAGE_SAMPLES_EXT
      // (0x906D)"; case 0x906E /* GL_IMAGE_BINDING_FORMAT_EXT */: return
      // "GL_IMAGE_BINDING_FORMAT_EXT (0x906E)"; case 0xFFFFFFFF /*
      // GL_ALL_BARRIER_BITS_EXT */: return "GL_ALL_BARRIER_BITS_EXT
      // (0xFFFFFFFF)";

      /* -------------------------- GL_EXT_shadow_funcs
       * -------------------------- */

      /* --------------------- GL_EXT_shared_texture_palette
       * --------------------- */

    case 0x81FB /* GL_SHARED_TEXTURE_PALETTE_EXT */:
      return "GL_SHARED_TEXTURE_PALETTE_EXT (0x81FB)";

      /* ------------------------ GL_EXT_stencil_clear_tag
       * ----------------------- */

    case 0x88F2 /* GL_STENCIL_TAG_BITS_EXT */:
      return "GL_STENCIL_TAG_BITS_EXT (0x88F2)";
    case 0x88F3 /* GL_STENCIL_CLEAR_TAG_VALUE_EXT */:
      return "GL_STENCIL_CLEAR_TAG_VALUE_EXT (0x88F3)";

      /* ------------------------ GL_EXT_stencil_two_side
       * ------------------------ */

    case 0x8910 /* GL_STENCIL_TEST_TWO_SIDE_EXT */:
      return "GL_STENCIL_TEST_TWO_SIDE_EXT (0x8910)";
    case 0x8911 /* GL_ACTIVE_STENCIL_FACE_EXT */:
      return "GL_ACTIVE_STENCIL_FACE_EXT (0x8911)"; /* --------------------------
                                                       GL_EXT_stencil_wrap
                                                       --------------------------
                                                     */

    // case 0x8507 /* GL_INCR_WRAP_EXT */: return "GL_INCR_WRAP_EXT (0x8507)";
    // case 0x8508 /* GL_DECR_WRAP_EXT */: return "GL_DECR_WRAP_EXT (0x8508)";

    /* --------------------------- GL_EXT_subtexture ---------------------------
     */

    /* ----------------------------- GL_EXT_texture ----------------------------
     */

    // case 0x803B /* GL_ALPHA4_EXT */: return "GL_ALPHA4_EXT (0x803B)";
    // case 0x803C /* GL_ALPHA8_EXT */: return "GL_ALPHA8_EXT (0x803C)";
    // case 0x803D /* GL_ALPHA12_EXT */: return "GL_ALPHA12_EXT (0x803D)";
    // case 0x803E /* GL_ALPHA16_EXT */: return "GL_ALPHA16_EXT (0x803E)";
    // case 0x803F /* GL_LUMINANCE4_EXT */: return "GL_LUMINANCE4_EXT (0x803F)";
    // case 0x8040 /* GL_LUMINANCE8_EXT */: return "GL_LUMINANCE8_EXT (0x8040)";
    // case 0x8041 /* GL_LUMINANCE12_EXT */: return "GL_LUMINANCE12_EXT
    // (0x8041)"; case 0x8042 /* GL_LUMINANCE16_EXT */: return
    // "GL_LUMINANCE16_EXT (0x8042)"; case 0x8043 /* GL_LUMINANCE4_ALPHA4_EXT */:
    // return "GL_LUMINANCE4_ALPHA4_EXT (0x8043)"; case 0x8044 /*
    // GL_LUMINANCE6_ALPHA2_EXT */: return "GL_LUMINANCE6_ALPHA2_EXT (0x8044)";
    // case 0x8045 /* GL_LUMINANCE8_ALPHA8_EXT */: return
    // "GL_LUMINANCE8_ALPHA8_EXT (0x8045)"; case 0x8046 /*
    // GL_LUMINANCE12_ALPHA4_EXT */: return "GL_LUMINANCE12_ALPHA4_EXT (0x8046)";
    // case 0x8047 /* GL_LUMINANCE12_ALPHA12_EXT */: return
    // "GL_LUMINANCE12_ALPHA12_EXT (0x8047)"; case 0x8048 /*
    // GL_LUMINANCE16_ALPHA16_EXT */: return "GL_LUMINANCE16_ALPHA16_EXT
    // (0x8048)"; case 0x8049 /* GL_INTENSITY_EXT */: return "GL_INTENSITY_EXT
    // (0x8049)"; case 0x804A /* GL_INTENSITY4_EXT */: return "GL_INTENSITY4_EXT
    // (0x804A)"; case 0x804B /* GL_INTENSITY8_EXT */: return "GL_INTENSITY8_EXT
    // (0x804B)"; case 0x804C /* GL_INTENSITY12_EXT */: return
    // "GL_INTENSITY12_EXT (0x804C)"; case 0x804D /* GL_INTENSITY16_EXT */:
    // return "GL_INTENSITY16_EXT (0x804D)";
    case 0x804E /* GL_RGB2_EXT */:
      return "GL_RGB2_EXT (0x804E)";
    // case 0x804F /* GL_RGB4_EXT */: return "GL_RGB4_EXT (0x804F)";
    // case 0x8050 /* GL_RGB5_EXT */: return "GL_RGB5_EXT (0x8050)";
    // case 0x8051 /* GL_RGB8_EXT */: return "GL_RGB8_EXT (0x8051)";
    // case 0x8052 /* GL_RGB10_EXT */: return "GL_RGB10_EXT (0x8052)";
    // case 0x8053 /* GL_RGB12_EXT */: return "GL_RGB12_EXT (0x8053)";
    // case 0x8054 /* GL_RGB16_EXT */: return "GL_RGB16_EXT (0x8054)";
    // case 0x8055 /* GL_RGBA2_EXT */: return "GL_RGBA2_EXT (0x8055)";
    // case 0x8056 /* GL_RGBA4_EXT */: return "GL_RGBA4_EXT (0x8056)";
    // case 0x8057 /* GL_RGB5_A1_EXT */: return "GL_RGB5_A1_EXT (0x8057)";
    // case 0x8058 /* GL_RGBA8_EXT */: return "GL_RGBA8_EXT (0x8058)";
    // case 0x8059 /* GL_RGB10_A2_EXT */: return "GL_RGB10_A2_EXT (0x8059)";
    // case 0x805A /* GL_RGBA12_EXT */: return "GL_RGBA12_EXT (0x805A)";
    // case 0x805B /* GL_RGBA16_EXT */: return "GL_RGBA16_EXT (0x805B)";
    // case 0x805C /* GL_TEXTURE_RED_SIZE_EXT */: return
    // "GL_TEXTURE_RED_SIZE_EXT (0x805C)"; case 0x805D /*
    // GL_TEXTURE_GREEN_SIZE_EXT */: return "GL_TEXTURE_GREEN_SIZE_EXT (0x805D)";
    // case 0x805E /* GL_TEXTURE_BLUE_SIZE_EXT */: return
    // "GL_TEXTURE_BLUE_SIZE_EXT (0x805E)"; case 0x805F /*
    // GL_TEXTURE_ALPHA_SIZE_EXT */: return "GL_TEXTURE_ALPHA_SIZE_EXT (0x805F)";
    // case 0x8060 /* GL_TEXTURE_LUMINANCE_SIZE_EXT */: return
    // "GL_TEXTURE_LUMINANCE_SIZE_EXT (0x8060)"; case 0x8061 /*
    // GL_TEXTURE_INTENSITY_SIZE_EXT */: return "GL_TEXTURE_INTENSITY_SIZE_EXT
    // (0x8061)";
    case 0x8062 /* GL_REPLACE_EXT */:
      return "GL_REPLACE_EXT (0x8062)";
      // case 0x8063 /* GL_PROXY_TEXTURE_1D_EXT */: return
      // "GL_PROXY_TEXTURE_1D_EXT (0x8063)"; case 0x8064 /*
      // GL_PROXY_TEXTURE_2D_EXT */: return "GL_PROXY_TEXTURE_2D_EXT (0x8064)";

      /* ---------------------------- GL_EXT_texture3D
       * --------------------------- */

      // case 0x806B /* GL_PACK_SKIP_IMAGES_EXT */: return
      // "GL_PACK_SKIP_IMAGES_EXT (0x806B)"; case 0x806C /*
      // GL_PACK_IMAGE_HEIGHT_EXT */: return "GL_PACK_IMAGE_HEIGHT_EXT (0x806C)";
      // case 0x806D /* GL_UNPACK_SKIP_IMAGES_EXT */: return
      // "GL_UNPACK_SKIP_IMAGES_EXT (0x806D)"; case 0x806E /*
      // GL_UNPACK_IMAGE_HEIGHT_EXT */: return "GL_UNPACK_IMAGE_HEIGHT_EXT
      // (0x806E)"; case 0x806F /* GL_TEXTURE_3D_EXT */: return
      // "GL_TEXTURE_3D_EXT (0x806F)"; case 0x8070 /* GL_PROXY_TEXTURE_3D_EXT */:
      // return "GL_PROXY_TEXTURE_3D_EXT (0x8070)"; case 0x8071 /*
      // GL_TEXTURE_DEPTH_EXT */: return "GL_TEXTURE_DEPTH_EXT (0x8071)"; case
      // 0x8072 /* GL_TEXTURE_WRAP_R_EXT */: return "GL_TEXTURE_WRAP_R_EXT
      // (0x8072)"; case 0x8073 /* GL_MAX_3D_TEXTURE_SIZE_EXT */: return
      // "GL_MAX_3D_TEXTURE_SIZE_EXT (0x8073)";/* --------------------------
      // GL_EXT_texture_array ------------------------- */

      // case 0x884E /* GL_COMPARE_REF_DEPTH_TO_TEXTURE_EXT */: return
      // "GL_COMPARE_REF_DEPTH_TO_TEXTURE_EXT (0x884E)"; case 0x88FF /*
      // GL_MAX_ARRAY_TEXTURE_LAYERS_EXT */: return
      // "GL_MAX_ARRAY_TEXTURE_LAYERS_EXT (0x88FF)"; case 0x8C18 /*
      // GL_TEXTURE_1D_ARRAY_EXT */: return "GL_TEXTURE_1D_ARRAY_EXT (0x8C18)";
      // case 0x8C19 /* GL_PROXY_TEXTURE_1D_ARRAY_EXT */: return
      // "GL_PROXY_TEXTURE_1D_ARRAY_EXT (0x8C19)"; case 0x8C1A /*
      // GL_TEXTURE_2D_ARRAY_EXT */: return "GL_TEXTURE_2D_ARRAY_EXT (0x8C1A)";
      // case 0x8C1B /* GL_PROXY_TEXTURE_2D_ARRAY_EXT */: return
      // "GL_PROXY_TEXTURE_2D_ARRAY_EXT (0x8C1B)"; case 0x8C1C /*
      // GL_TEXTURE_BINDING_1D_ARRAY_EXT */: return
      // "GL_TEXTURE_BINDING_1D_ARRAY_EXT (0x8C1C)"; case 0x8C1D /*
      // GL_TEXTURE_BINDING_2D_ARRAY_EXT */: return
      // "GL_TEXTURE_BINDING_2D_ARRAY_EXT (0x8C1D)";/* ----------------------
      // GL_EXT_texture_buffer_object --------------------- */

      // case 0x8C2A /* GL_TEXTURE_BUFFER_EXT */: return "GL_TEXTURE_BUFFER_EXT
      // (0x8C2A)"; case 0x8C2B /* GL_MAX_TEXTURE_BUFFER_SIZE_EXT */: return
      // "GL_MAX_TEXTURE_BUFFER_SIZE_EXT (0x8C2B)"; case 0x8C2C /*
      // GL_TEXTURE_BINDING_BUFFER_EXT */: return "GL_TEXTURE_BINDING_BUFFER_EXT
      // (0x8C2C)"; case 0x8C2D /* GL_TEXTURE_BUFFER_DATA_STORE_BINDING_EXT */:
      // return "GL_TEXTURE_BUFFER_DATA_STORE_BINDING_EXT (0x8C2D)"; case 0x8C2E
      // /* GL_TEXTURE_BUFFER_FORMAT_EXT */: return "GL_TEXTURE_BUFFER_FORMAT_EXT
      // (0x8C2E)";/* -------------------- GL_EXT_texture_compression_dxt1
      // -------------------- */

      /* -------------------- GL_EXT_texture_compression_latc
       * -------------------- */

    case 0x8C70 /* GL_COMPRESSED_LUMINANCE_LATC1_EXT */:
      return "GL_COMPRESSED_LUMINANCE_LATC1_EXT (0x8C70)";
    case 0x8C71 /* GL_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT */:
      return "GL_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT (0x8C71)";
    case 0x8C72 /* GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT */:
      return "GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT (0x8C72)";
    case 0x8C73 /* GL_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT */:
      return "GL_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT (0x8C73)";

      /* -------------------- GL_EXT_texture_compression_rgtc
       * -------------------- */

      // case 0x8DBB /* GL_COMPRESSED_RED_RGTC1_EXT */: return
      // "GL_COMPRESSED_RED_RGTC1_EXT (0x8DBB)"; case 0x8DBC /*
      // GL_COMPRESSED_SIGNED_RED_RGTC1_EXT */: return
      // "GL_COMPRESSED_SIGNED_RED_RGTC1_EXT (0x8DBC)"; case 0x8DBD /*
      // GL_COMPRESSED_RED_GREEN_RGTC2_EXT */: return
      // "GL_COMPRESSED_RED_GREEN_RGTC2_EXT (0x8DBD)"; case 0x8DBE /*
      // GL_COMPRESSED_SIGNED_RED_GREEN_RGTC2_EXT */: return
      // "GL_COMPRESSED_SIGNED_RED_GREEN_RGTC2_EXT (0x8DBE)";

      /* -------------------- GL_EXT_texture_compression_s3tc
       * -------------------- */

    case 0x83F0 /* GL_COMPRESSED_RGB_S3TC_DXT1_EXT */:
      return "GL_COMPRESSED_RGB_S3TC_DXT1_EXT (0x83F0)";
    case 0x83F1 /* GL_COMPRESSED_RGBA_S3TC_DXT1_EXT */:
      return "GL_COMPRESSED_RGBA_S3TC_DXT1_EXT (0x83F1)";
    case 0x83F2 /* GL_COMPRESSED_RGBA_S3TC_DXT3_EXT */:
      return "GL_COMPRESSED_RGBA_S3TC_DXT3_EXT (0x83F2)";
    case 0x83F3 /* GL_COMPRESSED_RGBA_S3TC_DXT5_EXT */:
      return "GL_COMPRESSED_RGBA_S3TC_DXT5_EXT (0x83F3)";

      /* ------------------------ GL_EXT_texture_cube_map
       * ------------------------ */

      // case 0x8511 /* GL_NORMAL_MAP_EXT */: return "GL_NORMAL_MAP_EXT
      // (0x8511)"; case 0x8512 /* GL_REFLECTION_MAP_EXT */: return
      // "GL_REFLECTION_MAP_EXT (0x8512)"; case 0x8513 /* GL_TEXTURE_CUBE_MAP_EXT
      // */: return "GL_TEXTURE_CUBE_MAP_EXT (0x8513)"; case 0x8514 /*
      // GL_TEXTURE_BINDING_CUBE_MAP_EXT */: return
      // "GL_TEXTURE_BINDING_CUBE_MAP_EXT (0x8514)"; case 0x8515 /*
      // GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT */: return
      // "GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT (0x8515)"; case 0x8516 /*
      // GL_TEXTURE_CUBE_MAP_NEGATIVE_X_EXT */: return
      // "GL_TEXTURE_CUBE_MAP_NEGATIVE_X_EXT (0x8516)"; case 0x8517 /*
      // GL_TEXTURE_CUBE_MAP_POSITIVE_Y_EXT */: return
      // "GL_TEXTURE_CUBE_MAP_POSITIVE_Y_EXT (0x8517)"; case 0x8518 /*
      // GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_EXT */: return
      // "GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_EXT (0x8518)"; case 0x8519 /*
      // GL_TEXTURE_CUBE_MAP_POSITIVE_Z_EXT */: return
      // "GL_TEXTURE_CUBE_MAP_POSITIVE_Z_EXT (0x8519)"; case 0x851A /*
      // GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_EXT */: return
      // "GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_EXT (0x851A)"; case 0x851B /*
      // GL_PROXY_TEXTURE_CUBE_MAP_EXT */: return "GL_PROXY_TEXTURE_CUBE_MAP_EXT
      // (0x851B)"; case 0x851C /* GL_MAX_CUBE_MAP_TEXTURE_SIZE_EXT */: return
      // "GL_MAX_CUBE_MAP_TEXTURE_SIZE_EXT (0x851C)";

      /* ----------------------- GL_EXT_texture_edge_clamp
       * ----------------------- */

      // case 0x812F /* GL_CLAMP_TO_EDGE_EXT */: return "GL_CLAMP_TO_EDGE_EXT
      // (0x812F)";

      /* --------------------------- GL_EXT_texture_env
       * -------------------------- */
      /* ------------------------- GL_EXT_texture_env_add
       * ------------------------ */

      /* ----------------------- GL_EXT_texture_env_combine
       * ---------------------- */

      // case 0x8570 /* GL_COMBINE_EXT */: return "GL_COMBINE_EXT (0x8570)";
      // case 0x8571 /* GL_COMBINE_RGB_EXT */: return "GL_COMBINE_RGB_EXT
      // (0x8571)"; case 0x8572 /* GL_COMBINE_ALPHA_EXT */: return
      // "GL_COMBINE_ALPHA_EXT (0x8572)"; case 0x8573 /* GL_RGB_SCALE_EXT */:
      // return "GL_RGB_SCALE_EXT (0x8573)"; case 0x8574 /* GL_ADD_SIGNED_EXT */:
      // return "GL_ADD_SIGNED_EXT (0x8574)";
      /// case 0x8575 /* GL_INTERPOLATE_EXT */: return "GL_INTERPOLATE_EXT
      /// (0x8575)";
      // case 0x8576 /* GL_CONSTANT_EXT */: return "GL_CONSTANT_EXT (0x8576)";
      // case 0x8577 /* GL_PRIMARY_COLOR_EXT */: return "GL_PRIMARY_COLOR_EXT
      // (0x8577)"; case 0x8578 /* GL_PREVIOUS_EXT */: return "GL_PREVIOUS_EXT
      // (0x8578)"; case 0x8580 /* GL_SOURCE0_RGB_EXT */: return
      // "GL_SOURCE0_RGB_EXT (0x8580)"; case 0x8581 /* GL_SOURCE1_RGB_EXT */:
      // return "GL_SOURCE1_RGB_EXT (0x8581)"; case 0x8582 /* GL_SOURCE2_RGB_EXT
      // */: return "GL_SOURCE2_RGB_EXT (0x8582)"; case 0x8588 /*
      // GL_SOURCE0_ALPHA_EXT */: return "GL_SOURCE0_ALPHA_EXT (0x8588)"; case
      // 0x8589 /* GL_SOURCE1_ALPHA_EXT */: return "GL_SOURCE1_ALPHA_EXT
      // (0x8589)"; case 0x858A /* GL_SOURCE2_ALPHA_EXT */: return
      // "GL_SOURCE2_ALPHA_EXT (0x858A)"; case 0x8590 /* GL_OPERAND0_RGB_EXT */:
      // return "GL_OPERAND0_RGB_EXT (0x8590)"; case 0x8591 /*
      // GL_OPERAND1_RGB_EXT */: return "GL_OPERAND1_RGB_EXT (0x8591)"; case
      // 0x8592 /* GL_OPERAND2_RGB_EXT */: return "GL_OPERAND2_RGB_EXT (0x8592)";
      // case 0x8598 /* GL_OPERAND0_ALPHA_EXT */: return "GL_OPERAND0_ALPHA_EXT
      // (0x8598)"; case 0x8599 /* GL_OPERAND1_ALPHA_EXT */: return
      // "GL_OPERAND1_ALPHA_EXT (0x8599)"; case 0x859A /* GL_OPERAND2_ALPHA_EXT
      // */: return "GL_OPERAND2_ALPHA_EXT (0x859A)";

      /* ------------------------ GL_EXT_texture_env_dot3
       * ------------------------ */

    case 0x8740 /* GL_DOT3_RGB_EXT */:
      return "GL_DOT3_RGB_EXT (0x8740)";
      // case 0x8741 /* GL_DOT3_RGBA_EXT */: return "GL_DOT3_RGBA_EXT (0x8741)";

      /* ------------------- GL_EXT_texture_filter_anisotropic
       * ------------------- */

    case 0x84FE /* GL_TEXTURE_MAX_ANISOTROPY_EXT */:
      return "GL_TEXTURE_MAX_ANISOTROPY_EXT (0x84FE)";
    case 0x84FF /* GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT */:
      return "GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT (0x84FF)";

    /* ------------------------- GL_EXT_texture_integer ------------------------
     */

    // case 0x8D70 /* GL_RGBA32UI_EXT */: return "GL_RGBA32UI_EXT (0x8D70)";
    // case 0x8D71 /* GL_RGB32UI_EXT */: return "GL_RGB32UI_EXT (0x8D71)";
    case 0x8D72 /* GL_ALPHA32UI_EXT */:
      return "GL_ALPHA32UI_EXT (0x8D72)";
    case 0x8D73 /* GL_INTENSITY32UI_EXT */:
      return "GL_INTENSITY32UI_EXT (0x8D73)";
    case 0x8D74 /* GL_LUMINANCE32UI_EXT */:
      return "GL_LUMINANCE32UI_EXT (0x8D74)";
    case 0x8D75 /* GL_LUMINANCE_ALPHA32UI_EXT */:
      return "GL_LUMINANCE_ALPHA32UI_EXT (0x8D75)";
    // case 0x8D76 /* GL_RGBA16UI_EXT */: return "GL_RGBA16UI_EXT (0x8D76)";
    // case 0x8D77 /* GL_RGB16UI_EXT */: return "GL_RGB16UI_EXT (0x8D77)";
    case 0x8D78 /* GL_ALPHA16UI_EXT */:
      return "GL_ALPHA16UI_EXT (0x8D78)";
    case 0x8D79 /* GL_INTENSITY16UI_EXT */:
      return "GL_INTENSITY16UI_EXT (0x8D79)";
    case 0x8D7A /* GL_LUMINANCE16UI_EXT */:
      return "GL_LUMINANCE16UI_EXT (0x8D7A)";
    case 0x8D7B /* GL_LUMINANCE_ALPHA16UI_EXT */:
      return "GL_LUMINANCE_ALPHA16UI_EXT (0x8D7B)";
    // case 0x8D7C /* GL_RGBA8UI_EXT */: return "GL_RGBA8UI_EXT (0x8D7C)";
    // case 0x8D7D /* GL_RGB8UI_EXT */: return "GL_RGB8UI_EXT (0x8D7D)";
    case 0x8D7E /* GL_ALPHA8UI_EXT */:
      return "GL_ALPHA8UI_EXT (0x8D7E)";
    case 0x8D7F /* GL_INTENSITY8UI_EXT */:
      return "GL_INTENSITY8UI_EXT (0x8D7F)";
    case 0x8D80 /* GL_LUMINANCE8UI_EXT */:
      return "GL_LUMINANCE8UI_EXT (0x8D80)";
    case 0x8D81 /* GL_LUMINANCE_ALPHA8UI_EXT */:
      return "GL_LUMINANCE_ALPHA8UI_EXT (0x8D81)";
    // case 0x8D82 /* GL_RGBA32I_EXT */: return "GL_RGBA32I_EXT (0x8D82)";
    // case 0x8D83 /* GL_RGB32I_EXT */: return "GL_RGB32I_EXT (0x8D83)";
    case 0x8D84 /* GL_ALPHA32I_EXT */:
      return "GL_ALPHA32I_EXT (0x8D84)";
    case 0x8D85 /* GL_INTENSITY32I_EXT */:
      return "GL_INTENSITY32I_EXT (0x8D85)";
    case 0x8D86 /* GL_LUMINANCE32I_EXT */:
      return "GL_LUMINANCE32I_EXT (0x8D86)";
    case 0x8D87 /* GL_LUMINANCE_ALPHA32I_EXT */:
      return "GL_LUMINANCE_ALPHA32I_EXT (0x8D87)";
    // case 0x8D88 /* GL_RGBA16I_EXT */: return "GL_RGBA16I_EXT (0x8D88)";
    // case 0x8D89 /* GL_RGB16I_EXT */: return "GL_RGB16I_EXT (0x8D89)";
    case 0x8D8A /* GL_ALPHA16I_EXT */:
      return "GL_ALPHA16I_EXT (0x8D8A)";
    case 0x8D8B /* GL_INTENSITY16I_EXT */:
      return "GL_INTENSITY16I_EXT (0x8D8B)";
    case 0x8D8C /* GL_LUMINANCE16I_EXT */:
      return "GL_LUMINANCE16I_EXT (0x8D8C)";
    case 0x8D8D /* GL_LUMINANCE_ALPHA16I_EXT */:
      return "GL_LUMINANCE_ALPHA16I_EXT (0x8D8D)";
    // case 0x8D8E /* GL_RGBA8I_EXT */: return "GL_RGBA8I_EXT (0x8D8E)";
    // case 0x8D8F /* GL_RGB8I_EXT */: return "GL_RGB8I_EXT (0x8D8F)";
    case 0x8D90 /* GL_ALPHA8I_EXT */:
      return "GL_ALPHA8I_EXT (0x8D90)";
    case 0x8D91 /* GL_INTENSITY8I_EXT */:
      return "GL_INTENSITY8I_EXT (0x8D91)";
    case 0x8D92 /* GL_LUMINANCE8I_EXT */:
      return "GL_LUMINANCE8I_EXT (0x8D92)";
    case 0x8D93 /* GL_LUMINANCE_ALPHA8I_EXT */:
      return "GL_LUMINANCE_ALPHA8I_EXT (0x8D93)";
    // case 0x8D94 /* GL_RED_INTEGER_EXT */: return "GL_RED_INTEGER_EXT
    // (0x8D94)"; case 0x8D95 /* GL_GREEN_INTEGER_EXT */: return
    // "GL_GREEN_INTEGER_EXT (0x8D95)"; case 0x8D96 /* GL_BLUE_INTEGER_EXT */:
    // return "GL_BLUE_INTEGER_EXT (0x8D96)"; case 0x8D97 /* GL_ALPHA_INTEGER_EXT
    // */: return "GL_ALPHA_INTEGER_EXT (0x8D97)"; case 0x8D98 /*
    // GL_RGB_INTEGER_EXT */: return "GL_RGB_INTEGER_EXT (0x8D98)"; case 0x8D99
    // /* GL_RGBA_INTEGER_EXT */: return "GL_RGBA_INTEGER_EXT (0x8D99)"; case
    // 0x8D9A /* GL_BGR_INTEGER_EXT */: return "GL_BGR_INTEGER_EXT (0x8D9A)";
    // case 0x8D9B /* GL_BGRA_INTEGER_EXT */: return "GL_BGRA_INTEGER_EXT
    // (0x8D9B)";
    case 0x8D9C /* GL_LUMINANCE_INTEGER_EXT */:
      return "GL_LUMINANCE_INTEGER_EXT (0x8D9C)";
    case 0x8D9D /* GL_LUMINANCE_ALPHA_INTEGER_EXT */:
      return "GL_LUMINANCE_ALPHA_INTEGER_EXT (0x8D9D)";
    case 0x8D9E /* GL_RGBA_INTEGER_MODE_EXT */:
      return "GL_RGBA_INTEGER_MODE_EXT (0x8D9E)"; /* ------------------------
                                                     GL_EXT_texture_lod_bias
                                                     ------------------------ */

    // case 0x84FD /* GL_MAX_TEXTURE_LOD_BIAS_EXT */: return
    // "GL_MAX_TEXTURE_LOD_BIAS_EXT (0x84FD)"; case 0x8500 /*
    // GL_TEXTURE_FILTER_CONTROL_EXT */: return "GL_TEXTURE_FILTER_CONTROL_EXT
    // (0x8500)"; case 0x8501 /* GL_TEXTURE_LOD_BIAS_EXT */: return
    // "GL_TEXTURE_LOD_BIAS_EXT (0x8501)";

    /* ---------------------- GL_EXT_texture_mirror_clamp ----------------------
     */

    // case 0x8742 /* GL_MIRROR_CLAMP_EXT */: return "GL_MIRROR_CLAMP_EXT
    // (0x8742)"; case 0x8743 /* GL_MIRROR_CLAMP_TO_EDGE_EXT */: return
    // "GL_MIRROR_CLAMP_TO_EDGE_EXT (0x8743)";
    case 0x8912 /* GL_MIRROR_CLAMP_TO_BORDER_EXT */:
      return "GL_MIRROR_CLAMP_TO_BORDER_EXT (0x8912)";

      /* ------------------------- GL_EXT_texture_object
       * ------------------------- */

      // case 0x8066 /* GL_TEXTURE_PRIORITY_EXT */: return
      // "GL_TEXTURE_PRIORITY_EXT (0x8066)"; case 0x8067 /*
      // GL_TEXTURE_RESIDENT_EXT */: return "GL_TEXTURE_RESIDENT_EXT (0x8067)";
      // case 0x8068 /* GL_TEXTURE_1D_BINDING_EXT */: return
      // "GL_TEXTURE_1D_BINDING_EXT (0x8068)"; case 0x8069 /*
      // GL_TEXTURE_2D_BINDING_EXT */: return "GL_TEXTURE_2D_BINDING_EXT
      // (0x8069)"; case 0x806A /* GL_TEXTURE_3D_BINDING_EXT */: return
      // "GL_TEXTURE_3D_BINDING_EXT (0x806A)";/* ---------------------
      // GL_EXT_texture_perturb_normal --------------------- */

    case 0x85AE /* GL_PERTURB_EXT */:
      return "GL_PERTURB_EXT (0x85AE)";
    case 0x85AF /* GL_TEXTURE_NORMAL_EXT */:
      return "GL_TEXTURE_NORMAL_EXT (0x85AF)"; /* ------------------------
                                                  GL_EXT_texture_rectangle
                                                  ----------------------- */

    // case 0x84F5 /* GL_TEXTURE_RECTANGLE_EXT */: return
    // "GL_TEXTURE_RECTANGLE_EXT (0x84F5)"; case 0x84F6 /*
    // GL_TEXTURE_BINDING_RECTANGLE_EXT */: return
    // "GL_TEXTURE_BINDING_RECTANGLE_EXT (0x84F6)"; case 0x84F7 /*
    // GL_PROXY_TEXTURE_RECTANGLE_EXT */: return "GL_PROXY_TEXTURE_RECTANGLE_EXT
    // (0x84F7)"; case 0x84F8 /* GL_MAX_RECTANGLE_TEXTURE_SIZE_EXT */: return
    // "GL_MAX_RECTANGLE_TEXTURE_SIZE_EXT (0x84F8)";

    /* -------------------------- GL_EXT_texture_sRGB --------------------------
     */

    // case 0x8C40 /* GL_SRGB_EXT */: return "GL_SRGB_EXT (0x8C40)";
    // case 0x8C41 /* GL_SRGB8_EXT */: return "GL_SRGB8_EXT (0x8C41)";
    // case 0x8C42 /* GL_SRGB_ALPHA_EXT */: return "GL_SRGB_ALPHA_EXT (0x8C42)";
    // case 0x8C43 /* GL_SRGB8_ALPHA8_EXT */: return "GL_SRGB8_ALPHA8_EXT
    // (0x8C43)"; case 0x8C44 /* GL_SLUMINANCE_ALPHA_EXT */: return
    // "GL_SLUMINANCE_ALPHA_EXT (0x8C44)"; case 0x8C45 /*
    // GL_SLUMINANCE8_ALPHA8_EXT */: return "GL_SLUMINANCE8_ALPHA8_EXT (0x8C45)";
    // case 0x8C46 /* GL_SLUMINANCE_EXT */: return "GL_SLUMINANCE_EXT (0x8C46)";
    // case 0x8C47 /* GL_SLUMINANCE8_EXT */: return "GL_SLUMINANCE8_EXT
    // (0x8C47)"; case 0x8C48 /* GL_COMPRESSED_SRGB_EXT */: return
    // "GL_COMPRESSED_SRGB_EXT (0x8C48)"; case 0x8C49 /*
    // GL_COMPRESSED_SRGB_ALPHA_EXT */: return "GL_COMPRESSED_SRGB_ALPHA_EXT
    // (0x8C49)"; case 0x8C4A /* GL_COMPRESSED_SLUMINANCE_EXT */: return
    // "GL_COMPRESSED_SLUMINANCE_EXT (0x8C4A)"; case 0x8C4B /*
    // GL_COMPRESSED_SLUMINANCE_ALPHA_EXT */: return
    // "GL_COMPRESSED_SLUMINANCE_ALPHA_EXT (0x8C4B)";
    case 0x8C4C /* GL_COMPRESSED_SRGB_S3TC_DXT1_EXT */:
      return "GL_COMPRESSED_SRGB_S3TC_DXT1_EXT (0x8C4C)";
    case 0x8C4D /* GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT */:
      return "GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT (0x8C4D)";
    case 0x8C4E /* GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT */:
      return "GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT (0x8C4E)";
    case 0x8C4F /* GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT */:
      return "GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT (0x8C4F)";

      /* ----------------------- GL_EXT_texture_sRGB_decode
       * ---------------------- */

    case 0x8A48 /* GL_TEXTURE_SRGB_DECODE_EXT */:
      return "GL_TEXTURE_SRGB_DECODE_EXT (0x8A48)";
    case 0x8A49 /* GL_DECODE_EXT */:
      return "GL_DECODE_EXT (0x8A49)";
    case 0x8A4A /* GL_SKIP_DECODE_EXT */:
      return "GL_SKIP_DECODE_EXT (0x8A4A)";

    /* --------------------- GL_EXT_texture_shared_exponent --------------------
     */

    // case 0x8C3D /* GL_RGB9_E5_EXT */: return "GL_RGB9_E5_EXT (0x8C3D)";
    // case 0x8C3E /* GL_UNSIGNED_INT_5_9_9_9_REV_EXT */: return
    // "GL_UNSIGNED_INT_5_9_9_9_REV_EXT (0x8C3E)"; case 0x8C3F /*
    // GL_TEXTURE_SHARED_SIZE_EXT */: return "GL_TEXTURE_SHARED_SIZE_EXT
    // (0x8C3F)";

    /* -------------------------- GL_EXT_texture_snorm -------------------------
     */

    // case 0x8F90 /* GL_RED_SNORM */: return "GL_RED_SNORM (0x8F90)";
    // case 0x8F91 /* GL_RG_SNORM */: return "GL_RG_SNORM (0x8F91)";
    // case 0x8F92 /* GL_RGB_SNORM */: return "GL_RGB_SNORM (0x8F92)";
    // case 0x8F93 /* GL_RGBA_SNORM */: return "GL_RGBA_SNORM (0x8F93)";
    // case 0x8F94 /* GL_R8_SNORM */: return "GL_R8_SNORM (0x8F94)";
    // case 0x8F95 /* GL_RG8_SNORM */: return "GL_RG8_SNORM (0x8F95)";
    // case 0x8F96 /* GL_RGB8_SNORM */: return "GL_RGB8_SNORM (0x8F96)";
    // case 0x8F97 /* GL_RGBA8_SNORM */: return "GL_RGBA8_SNORM (0x8F97)";
    // case 0x8F98 /* GL_R16_SNORM */: return "GL_R16_SNORM (0x8F98)";
    // case 0x8F99 /* GL_RG16_SNORM */: return "GL_RG16_SNORM (0x8F99)";
    // case 0x8F9A /* GL_RGB16_SNORM */: return "GL_RGB16_SNORM (0x8F9A)";
    // case 0x8F9B /* GL_RGBA16_SNORM */: return "GL_RGBA16_SNORM (0x8F9B)";
    // case 0x8F9C /* GL_SIGNED_NORMALIZED */: return "GL_SIGNED_NORMALIZED
    // (0x8F9C)";
    case 0x9010 /* GL_ALPHA_SNORM */:
      return "GL_ALPHA_SNORM (0x9010)";
    case 0x9011 /* GL_LUMINANCE_SNORM */:
      return "GL_LUMINANCE_SNORM (0x9011)";
    case 0x9012 /* GL_LUMINANCE_ALPHA_SNORM */:
      return "GL_LUMINANCE_ALPHA_SNORM (0x9012)";
    case 0x9013 /* GL_INTENSITY_SNORM */:
      return "GL_INTENSITY_SNORM (0x9013)";
    case 0x9014 /* GL_ALPHA8_SNORM */:
      return "GL_ALPHA8_SNORM (0x9014)";
    case 0x9015 /* GL_LUMINANCE8_SNORM */:
      return "GL_LUMINANCE8_SNORM (0x9015)";
    case 0x9016 /* GL_LUMINANCE8_ALPHA8_SNORM */:
      return "GL_LUMINANCE8_ALPHA8_SNORM (0x9016)";
    case 0x9017 /* GL_INTENSITY8_SNORM */:
      return "GL_INTENSITY8_SNORM (0x9017)";
    case 0x9018 /* GL_ALPHA16_SNORM */:
      return "GL_ALPHA16_SNORM (0x9018)";
    case 0x9019 /* GL_LUMINANCE16_SNORM */:
      return "GL_LUMINANCE16_SNORM (0x9019)";
    case 0x901A /* GL_LUMINANCE16_ALPHA16_SNORM */:
      return "GL_LUMINANCE16_ALPHA16_SNORM (0x901A)";
    case 0x901B /* GL_INTENSITY16_SNORM */:
      return "GL_INTENSITY16_SNORM (0x901B)";

    /* ------------------------- GL_EXT_texture_swizzle ------------------------
     */

    // case 0x8E42 /* GL_TEXTURE_SWIZZLE_R_EXT */: return
    // "GL_TEXTURE_SWIZZLE_R_EXT (0x8E42)"; case 0x8E43 /*
    // GL_TEXTURE_SWIZZLE_G_EXT */: return "GL_TEXTURE_SWIZZLE_G_EXT (0x8E43)";
    // case 0x8E44 /* GL_TEXTURE_SWIZZLE_B_EXT */: return
    // "GL_TEXTURE_SWIZZLE_B_EXT (0x8E44)"; case 0x8E45 /*
    // GL_TEXTURE_SWIZZLE_A_EXT */: return "GL_TEXTURE_SWIZZLE_A_EXT (0x8E45)";
    // case 0x8E46 /* GL_TEXTURE_SWIZZLE_RGBA_EXT */: return
    // "GL_TEXTURE_SWIZZLE_RGBA_EXT (0x8E46)";

    /* --------------------------- GL_EXT_timer_query --------------------------
     */

    // case 0x88BF /* GL_TIME_ELAPSED_EXT */: return "GL_TIME_ELAPSED_EXT
    // (0x88BF)";

    /* ----------------------- GL_EXT_transform_feedback -----------------------
     */

    // case 0x8C76 /* GL_TRANSFORM_FEEDBACK_VARYING_MAX_LENGTH_EXT */: return
    // "GL_TRANSFORM_FEEDBACK_VARYING_MAX_LENGTH_EXT (0x8C76)"; case 0x8C7F /*
    // GL_TRANSFORM_FEEDBACK_BUFFER_MODE_EXT */: return
    // "GL_TRANSFORM_FEEDBACK_BUFFER_MODE_EXT (0x8C7F)";
    /// case 0x8C80 /* GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS_EXT */:
    /// return "GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS_EXT (0x8C80)";
    // case 0x8C83 /* GL_TRANSFORM_FEEDBACK_VARYINGS_EXT */: return
    // "GL_TRANSFORM_FEEDBACK_VARYINGS_EXT (0x8C83)"; case 0x8C84 /*
    // GL_TRANSFORM_FEEDBACK_BUFFER_START_EXT */: return
    // "GL_TRANSFORM_FEEDBACK_BUFFER_START_EXT (0x8C84)"; case 0x8C85 /*
    // GL_TRANSFORM_FEEDBACK_BUFFER_SIZE_EXT */: return
    // "GL_TRANSFORM_FEEDBACK_BUFFER_SIZE_EXT (0x8C85)"; case 0x8C87 /*
    // GL_PRIMITIVES_GENERATED_EXT */: return "GL_PRIMITIVES_GENERATED_EXT
    // (0x8C87)"; case 0x8C88 /* GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN_EXT */:
    // return "GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN_EXT (0x8C88)"; case
    // 0x8C89 /* GL_RASTERIZER_DISCARD_EXT */: return "GL_RASTERIZER_DISCARD_EXT
    // (0x8C89)"; case 0x8C8A /*
    // GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS_EXT */: return
    // "GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS_EXT (0x8C8A)"; case
    // 0x8C8B /* GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS_EXT */: return
    // "GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS_EXT (0x8C8B)"; case 0x8C8C /*
    // GL_INTERLEAVED_ATTRIBS_EXT */: return "GL_INTERLEAVED_ATTRIBS_EXT
    // (0x8C8C)"; case 0x8C8D /* GL_SEPARATE_ATTRIBS_EXT */: return
    // "GL_SEPARATE_ATTRIBS_EXT (0x8C8D)"; case 0x8C8E /*
    // GL_TRANSFORM_FEEDBACK_BUFFER_EXT */: return
    // "GL_TRANSFORM_FEEDBACK_BUFFER_EXT (0x8C8E)"; case 0x8C8F /*
    // GL_TRANSFORM_FEEDBACK_BUFFER_BINDING_EXT */: return
    // "GL_TRANSFORM_FEEDBACK_BUFFER_BINDING_EXT (0x8C8F)";

    /* -------------------------- GL_EXT_vertex_array --------------------------
     */

    // case 0x140A /* GL_DOUBLE_EXT */: return "GL_DOUBLE_EXT (0x140A)";
    // case 0x8074 /* GL_VERTEX_ARRAY_EXT */: return "GL_VERTEX_ARRAY_EXT
    // (0x8074)"; case 0x8075 /* GL_NORMAL_ARRAY_EXT */: return
    // "GL_NORMAL_ARRAY_EXT (0x8075)"; case 0x8076 /* GL_COLOR_ARRAY_EXT */:
    // return "GL_COLOR_ARRAY_EXT (0x8076)"; case 0x8077 /* GL_INDEX_ARRAY_EXT
    // */: return "GL_INDEX_ARRAY_EXT (0x8077)"; case 0x8078 /*
    // GL_TEXTURE_COORD_ARRAY_EXT */: return "GL_TEXTURE_COORD_ARRAY_EXT
    // (0x8078)"; case 0x8079 /* GL_EDGE_FLAG_ARRAY_EXT */: return
    // "GL_EDGE_FLAG_ARRAY_EXT (0x8079)"; case 0x807A /* GL_VERTEX_ARRAY_SIZE_EXT
    // */: return "GL_VERTEX_ARRAY_SIZE_EXT (0x807A)"; case 0x807B /*
    // GL_VERTEX_ARRAY_TYPE_EXT */: return "GL_VERTEX_ARRAY_TYPE_EXT (0x807B)";
    // case 0x807C /* GL_VERTEX_ARRAY_STRIDE_EXT */: return
    // "GL_VERTEX_ARRAY_STRIDE_EXT (0x807C)";
    case 0x807D /* GL_VERTEX_ARRAY_COUNT_EXT */:
      return "GL_VERTEX_ARRAY_COUNT_EXT (0x807D)";
    // case 0x807E /* GL_NORMAL_ARRAY_TYPE_EXT */: return
    // "GL_NORMAL_ARRAY_TYPE_EXT (0x807E)"; case 0x807F /*
    // GL_NORMAL_ARRAY_STRIDE_EXT */: return "GL_NORMAL_ARRAY_STRIDE_EXT
    // (0x807F)";
    case 0x8080 /* GL_NORMAL_ARRAY_COUNT_EXT */:
      return "GL_NORMAL_ARRAY_COUNT_EXT (0x8080)";
    // case 0x8081 /* GL_COLOR_ARRAY_SIZE_EXT */: return
    // "GL_COLOR_ARRAY_SIZE_EXT (0x8081)"; case 0x8082 /* GL_COLOR_ARRAY_TYPE_EXT
    // */: return "GL_COLOR_ARRAY_TYPE_EXT (0x8082)"; case 0x8083 /*
    // GL_COLOR_ARRAY_STRIDE_EXT */: return "GL_COLOR_ARRAY_STRIDE_EXT (0x8083)";
    case 0x8084 /* GL_COLOR_ARRAY_COUNT_EXT */:
      return "GL_COLOR_ARRAY_COUNT_EXT (0x8084)";
    // case 0x8085 /* GL_INDEX_ARRAY_TYPE_EXT */: return
    // "GL_INDEX_ARRAY_TYPE_EXT (0x8085)"; case 0x8086 /*
    // GL_INDEX_ARRAY_STRIDE_EXT */: return "GL_INDEX_ARRAY_STRIDE_EXT (0x8086)";
    case 0x8087 /* GL_INDEX_ARRAY_COUNT_EXT */:
      return "GL_INDEX_ARRAY_COUNT_EXT (0x8087)";
    // case 0x8088 /* GL_TEXTURE_COORD_ARRAY_SIZE_EXT */: return
    // "GL_TEXTURE_COORD_ARRAY_SIZE_EXT (0x8088)"; case 0x8089 /*
    // GL_TEXTURE_COORD_ARRAY_TYPE_EXT */: return
    // "GL_TEXTURE_COORD_ARRAY_TYPE_EXT (0x8089)"; case 0x808A /*
    // GL_TEXTURE_COORD_ARRAY_STRIDE_EXT */: return
    // "GL_TEXTURE_COORD_ARRAY_STRIDE_EXT (0x808A)";
    case 0x808B /* GL_TEXTURE_COORD_ARRAY_COUNT_EXT */:
      return "GL_TEXTURE_COORD_ARRAY_COUNT_EXT (0x808B)";
    // case 0x808C /* GL_EDGE_FLAG_ARRAY_STRIDE_EXT */: return
    // "GL_EDGE_FLAG_ARRAY_STRIDE_EXT (0x808C)";
    case 0x808D /* GL_EDGE_FLAG_ARRAY_COUNT_EXT */:
      return "GL_EDGE_FLAG_ARRAY_COUNT_EXT (0x808D)";
      // case 0x808E /* GL_VERTEX_ARRAY_POINTER_EXT */: return
      // "GL_VERTEX_ARRAY_POINTER_EXT (0x808E)"; case 0x808F /*
      // GL_NORMAL_ARRAY_POINTER_EXT */: return "GL_NORMAL_ARRAY_POINTER_EXT
      // (0x808F)"; case 0x8090 /* GL_COLOR_ARRAY_POINTER_EXT */: return
      // "GL_COLOR_ARRAY_POINTER_EXT (0x8090)";
      /// case 0x8091 /* GL_INDEX_ARRAY_POINTER_EXT */: return
      /// "GL_INDEX_ARRAY_POINTER_EXT (0x8091)"; case 0x8092 /*
      /// GL_TEXTURE_COORD_ARRAY_POINTER_EXT */: return
      /// "GL_TEXTURE_COORD_ARRAY_POINTER_EXT (0x8092)";
      // case 0x8093 /* GL_EDGE_FLAG_ARRAY_POINTER_EXT */: return
      // "GL_EDGE_FLAG_ARRAY_POINTER_EXT (0x8093)";

      /* ------------------------ GL_EXT_vertex_array_bgra
       * ----------------------- */

      // case 0x80E1 /* GL_BGRA */: return "GL_BGRA (0x80E1)";

      /* ----------------------- GL_EXT_vertex_attrib_64bit
       * ---------------------- */

      // case 0x8F46 /* GL_DOUBLE_MAT2_EXT */: return "GL_DOUBLE_MAT2_EXT
      // (0x8F46)"; case 0x8F47 /* GL_DOUBLE_MAT3_EXT */: return
      // "GL_DOUBLE_MAT3_EXT (0x8F47)"; case 0x8F48 /* GL_DOUBLE_MAT4_EXT */:
      // return "GL_DOUBLE_MAT4_EXT (0x8F48)"; case 0x8F49 /*
      // GL_DOUBLE_MAT2x3_EXT */: return "GL_DOUBLE_MAT2x3_EXT (0x8F49)"; case
      // 0x8F4A /* GL_DOUBLE_MAT2x4_EXT */: return "GL_DOUBLE_MAT2x4_EXT
      // (0x8F4A)"; case 0x8F4B /* GL_DOUBLE_MAT3x2_EXT */: return
      // "GL_DOUBLE_MAT3x2_EXT (0x8F4B)"; case 0x8F4C /* GL_DOUBLE_MAT3x4_EXT */:
      // return "GL_DOUBLE_MAT3x4_EXT (0x8F4C)"; case 0x8F4D /*
      // GL_DOUBLE_MAT4x2_EXT */: return "GL_DOUBLE_MAT4x2_EXT (0x8F4D)"; case
      // 0x8F4E /* GL_DOUBLE_MAT4x3_EXT */: return "GL_DOUBLE_MAT4x3_EXT
      // (0x8F4E)"; case 0x8FFC /* GL_DOUBLE_VEC2_EXT */: return
      // "GL_DOUBLE_VEC2_EXT (0x8FFC)"; case 0x8FFD /* GL_DOUBLE_VEC3_EXT */:
      // return "GL_DOUBLE_VEC3_EXT (0x8FFD)"; case 0x8FFE /* GL_DOUBLE_VEC4_EXT
      // */: return "GL_DOUBLE_VEC4_EXT (0x8FFE)";/* --------------------------
      // GL_EXT_vertex_shader ------------------------- */

    case 0x8780 /* GL_VERTEX_SHADER_EXT */:
      return "GL_VERTEX_SHADER_EXT (0x8780)";
    case 0x8781 /* GL_VERTEX_SHADER_BINDING_EXT */:
      return "GL_VERTEX_SHADER_BINDING_EXT (0x8781)";
    case 0x8782 /* GL_OP_INDEX_EXT */:
      return "GL_OP_INDEX_EXT (0x8782)";
    case 0x8783 /* GL_OP_NEGATE_EXT */:
      return "GL_OP_NEGATE_EXT (0x8783)";
    case 0x8784 /* GL_OP_DOT3_EXT */:
      return "GL_OP_DOT3_EXT (0x8784)";
    case 0x8785 /* GL_OP_DOT4_EXT */:
      return "GL_OP_DOT4_EXT (0x8785)";
    case 0x8786 /* GL_OP_MUL_EXT */:
      return "GL_OP_MUL_EXT (0x8786)";
    case 0x8787 /* GL_OP_ADD_EXT */:
      return "GL_OP_ADD_EXT (0x8787)";
    case 0x8788 /* GL_OP_MADD_EXT */:
      return "GL_OP_MADD_EXT (0x8788)";
    case 0x8789 /* GL_OP_FRAC_EXT */:
      return "GL_OP_FRAC_EXT (0x8789)";
    case 0x878A /* GL_OP_MAX_EXT */:
      return "GL_OP_MAX_EXT (0x878A)";
    case 0x878B /* GL_OP_MIN_EXT */:
      return "GL_OP_MIN_EXT (0x878B)";
    case 0x878C /* GL_OP_SET_GE_EXT */:
      return "GL_OP_SET_GE_EXT (0x878C)";
    case 0x878D /* GL_OP_SET_LT_EXT */:
      return "GL_OP_SET_LT_EXT (0x878D)";
    case 0x878E /* GL_OP_CLAMP_EXT */:
      return "GL_OP_CLAMP_EXT (0x878E)";
    case 0x878F /* GL_OP_FLOOR_EXT */:
      return "GL_OP_FLOOR_EXT (0x878F)";
    case 0x8790 /* GL_OP_ROUND_EXT */:
      return "GL_OP_ROUND_EXT (0x8790)";
    case 0x8791 /* GL_OP_EXP_BASE_2_EXT */:
      return "GL_OP_EXP_BASE_2_EXT (0x8791)";
    case 0x8792 /* GL_OP_LOG_BASE_2_EXT */:
      return "GL_OP_LOG_BASE_2_EXT (0x8792)";
    case 0x8793 /* GL_OP_POWER_EXT */:
      return "GL_OP_POWER_EXT (0x8793)";
    case 0x8794 /* GL_OP_RECIP_EXT */:
      return "GL_OP_RECIP_EXT (0x8794)";
    case 0x8795 /* GL_OP_RECIP_SQRT_EXT */:
      return "GL_OP_RECIP_SQRT_EXT (0x8795)";
    case 0x8796 /* GL_OP_SUB_EXT */:
      return "GL_OP_SUB_EXT (0x8796)";
    case 0x8797 /* GL_OP_CROSS_PRODUCT_EXT */:
      return "GL_OP_CROSS_PRODUCT_EXT (0x8797)";
    case 0x8798 /* GL_OP_MULTIPLY_MATRIX_EXT */:
      return "GL_OP_MULTIPLY_MATRIX_EXT (0x8798)";
    case 0x8799 /* GL_OP_MOV_EXT */:
      return "GL_OP_MOV_EXT (0x8799)";
    case 0x879A /* GL_OUTPUT_VERTEX_EXT */:
      return "GL_OUTPUT_VERTEX_EXT (0x879A)";
    case 0x879B /* GL_OUTPUT_COLOR0_EXT */:
      return "GL_OUTPUT_COLOR0_EXT (0x879B)";
    case 0x879C /* GL_OUTPUT_COLOR1_EXT */:
      return "GL_OUTPUT_COLOR1_EXT (0x879C)";
    case 0x879D /* GL_OUTPUT_TEXTURE_COORD0_EXT */:
      return "GL_OUTPUT_TEXTURE_COORD0_EXT (0x879D)";
    case 0x879E /* GL_OUTPUT_TEXTURE_COORD1_EXT */:
      return "GL_OUTPUT_TEXTURE_COORD1_EXT (0x879E)";
    case 0x879F /* GL_OUTPUT_TEXTURE_COORD2_EXT */:
      return "GL_OUTPUT_TEXTURE_COORD2_EXT (0x879F)";
    case 0x87A0 /* GL_OUTPUT_TEXTURE_COORD3_EXT */:
      return "GL_OUTPUT_TEXTURE_COORD3_EXT (0x87A0)";
    case 0x87A1 /* GL_OUTPUT_TEXTURE_COORD4_EXT */:
      return "GL_OUTPUT_TEXTURE_COORD4_EXT (0x87A1)";
    case 0x87A2 /* GL_OUTPUT_TEXTURE_COORD5_EXT */:
      return "GL_OUTPUT_TEXTURE_COORD5_EXT (0x87A2)";
    case 0x87A3 /* GL_OUTPUT_TEXTURE_COORD6_EXT */:
      return "GL_OUTPUT_TEXTURE_COORD6_EXT (0x87A3)";
    case 0x87A4 /* GL_OUTPUT_TEXTURE_COORD7_EXT */:
      return "GL_OUTPUT_TEXTURE_COORD7_EXT (0x87A4)";
    case 0x87A5 /* GL_OUTPUT_TEXTURE_COORD8_EXT */:
      return "GL_OUTPUT_TEXTURE_COORD8_EXT (0x87A5)";
    case 0x87A6 /* GL_OUTPUT_TEXTURE_COORD9_EXT */:
      return "GL_OUTPUT_TEXTURE_COORD9_EXT (0x87A6)";
    case 0x87A7 /* GL_OUTPUT_TEXTURE_COORD10_EXT */:
      return "GL_OUTPUT_TEXTURE_COORD10_EXT (0x87A7)";
    case 0x87A8 /* GL_OUTPUT_TEXTURE_COORD11_EXT */:
      return "GL_OUTPUT_TEXTURE_COORD11_EXT (0x87A8)";
    case 0x87A9 /* GL_OUTPUT_TEXTURE_COORD12_EXT */:
      return "GL_OUTPUT_TEXTURE_COORD12_EXT (0x87A9)";
    case 0x87AA /* GL_OUTPUT_TEXTURE_COORD13_EXT */:
      return "GL_OUTPUT_TEXTURE_COORD13_EXT (0x87AA)";
    case 0x87AB /* GL_OUTPUT_TEXTURE_COORD14_EXT */:
      return "GL_OUTPUT_TEXTURE_COORD14_EXT (0x87AB)";
    case 0x87AC /* GL_OUTPUT_TEXTURE_COORD15_EXT */:
      return "GL_OUTPUT_TEXTURE_COORD15_EXT (0x87AC)";
    case 0x87AD /* GL_OUTPUT_TEXTURE_COORD16_EXT */:
      return "GL_OUTPUT_TEXTURE_COORD16_EXT (0x87AD)";
    case 0x87AE /* GL_OUTPUT_TEXTURE_COORD17_EXT */:
      return "GL_OUTPUT_TEXTURE_COORD17_EXT (0x87AE)";
    case 0x87AF /* GL_OUTPUT_TEXTURE_COORD18_EXT */:
      return "GL_OUTPUT_TEXTURE_COORD18_EXT (0x87AF)";
    case 0x87B0 /* GL_OUTPUT_TEXTURE_COORD19_EXT */:
      return "GL_OUTPUT_TEXTURE_COORD19_EXT (0x87B0)";
    case 0x87B1 /* GL_OUTPUT_TEXTURE_COORD20_EXT */:
      return "GL_OUTPUT_TEXTURE_COORD20_EXT (0x87B1)";
    case 0x87B2 /* GL_OUTPUT_TEXTURE_COORD21_EXT */:
      return "GL_OUTPUT_TEXTURE_COORD21_EXT (0x87B2)";
    case 0x87B3 /* GL_OUTPUT_TEXTURE_COORD22_EXT */:
      return "GL_OUTPUT_TEXTURE_COORD22_EXT (0x87B3)";
    case 0x87B4 /* GL_OUTPUT_TEXTURE_COORD23_EXT */:
      return "GL_OUTPUT_TEXTURE_COORD23_EXT (0x87B4)";
    case 0x87B5 /* GL_OUTPUT_TEXTURE_COORD24_EXT */:
      return "GL_OUTPUT_TEXTURE_COORD24_EXT (0x87B5)";
    case 0x87B6 /* GL_OUTPUT_TEXTURE_COORD25_EXT */:
      return "GL_OUTPUT_TEXTURE_COORD25_EXT (0x87B6)";
    case 0x87B7 /* GL_OUTPUT_TEXTURE_COORD26_EXT */:
      return "GL_OUTPUT_TEXTURE_COORD26_EXT (0x87B7)";
    case 0x87B8 /* GL_OUTPUT_TEXTURE_COORD27_EXT */:
      return "GL_OUTPUT_TEXTURE_COORD27_EXT (0x87B8)";
    case 0x87B9 /* GL_OUTPUT_TEXTURE_COORD28_EXT */:
      return "GL_OUTPUT_TEXTURE_COORD28_EXT (0x87B9)";
    case 0x87BA /* GL_OUTPUT_TEXTURE_COORD29_EXT */:
      return "GL_OUTPUT_TEXTURE_COORD29_EXT (0x87BA)";
    case 0x87BB /* GL_OUTPUT_TEXTURE_COORD30_EXT */:
      return "GL_OUTPUT_TEXTURE_COORD30_EXT (0x87BB)";
    case 0x87BC /* GL_OUTPUT_TEXTURE_COORD31_EXT */:
      return "GL_OUTPUT_TEXTURE_COORD31_EXT (0x87BC)";
    case 0x87BD /* GL_OUTPUT_FOG_EXT */:
      return "GL_OUTPUT_FOG_EXT (0x87BD)";
    case 0x87BE /* GL_SCALAR_EXT */:
      return "GL_SCALAR_EXT (0x87BE)";
    case 0x87BF /* GL_VECTOR_EXT */:
      return "GL_VECTOR_EXT (0x87BF)";
    case 0x87C0 /* GL_MATRIX_EXT */:
      return "GL_MATRIX_EXT (0x87C0)";
    case 0x87C1 /* GL_VARIANT_EXT */:
      return "GL_VARIANT_EXT (0x87C1)";
    case 0x87C2 /* GL_INVARIANT_EXT */:
      return "GL_INVARIANT_EXT (0x87C2)";
    case 0x87C3 /* GL_LOCAL_CONSTANT_EXT */:
      return "GL_LOCAL_CONSTANT_EXT (0x87C3)";
    case 0x87C4 /* GL_LOCAL_EXT */:
      return "GL_LOCAL_EXT (0x87C4)";
    case 0x87C6 /* GL_MAX_VERTEX_SHADER_VARIANTS_EXT */:
      return "GL_MAX_VERTEX_SHADER_VARIANTS_EXT (0x87C6)";
    case 0x87C7 /* GL_MAX_VERTEX_SHADER_INVARIANTS_EXT */:
      return "GL_MAX_VERTEX_SHADER_INVARIANTS_EXT (0x87C7)";
    case 0x87C8 /* GL_MAX_VERTEX_SHADER_LOCAL_CONSTANTS_EXT */:
      return "GL_MAX_VERTEX_SHADER_LOCAL_CONSTANTS_EXT (0x87C8)";
    case 0x87C9 /* GL_MAX_VERTEX_SHADER_LOCALS_EXT */:
      return "GL_MAX_VERTEX_SHADER_LOCALS_EXT (0x87C9)";
    case 0x87CB /* GL_MAX_OPTIMIZED_VERTEX_SHADER_VARIANTS_EXT */:
      return "GL_MAX_OPTIMIZED_VERTEX_SHADER_VARIANTS_EXT (0x87CB)";
    case 0x87CC /* GL_MAX_OPTIMIZED_VERTEX_SHADER_INVARIANTS_EXT */:
      return "GL_MAX_OPTIMIZED_VERTEX_SHADER_INVARIANTS_EXT (0x87CC)";
    case 0x87CD /* GL_MAX_OPTIMIZED_VERTEX_SHADER_LOCAL_CONSTANTS_EXT */:
      return "GL_MAX_OPTIMIZED_VERTEX_SHADER_LOCAL_CONSTANTS_EXT (0x87CD)";
    case 0x87CE /* GL_MAX_OPTIMIZED_VERTEX_SHADER_LOCALS_EXT */:
      return "GL_MAX_OPTIMIZED_VERTEX_SHADER_LOCALS_EXT (0x87CE)";
    case 0x87D0 /* GL_VERTEX_SHADER_VARIANTS_EXT */:
      return "GL_VERTEX_SHADER_VARIANTS_EXT (0x87D0)";
    case 0x87D1 /* GL_VERTEX_SHADER_INVARIANTS_EXT */:
      return "GL_VERTEX_SHADER_INVARIANTS_EXT (0x87D1)";
    case 0x87D2 /* GL_VERTEX_SHADER_LOCAL_CONSTANTS_EXT */:
      return "GL_VERTEX_SHADER_LOCAL_CONSTANTS_EXT (0x87D2)";
    case 0x87D3 /* GL_VERTEX_SHADER_LOCALS_EXT */:
      return "GL_VERTEX_SHADER_LOCALS_EXT (0x87D3)";
    case 0x87D4 /* GL_VERTEX_SHADER_OPTIMIZED_EXT */:
      return "GL_VERTEX_SHADER_OPTIMIZED_EXT (0x87D4)";
    case 0x87D5 /* GL_X_EXT */:
      return "GL_X_EXT (0x87D5)";
    case 0x87D6 /* GL_Y_EXT */:
      return "GL_Y_EXT (0x87D6)";
    case 0x87D7 /* GL_Z_EXT */:
      return "GL_Z_EXT (0x87D7)";
    case 0x87D8 /* GL_W_EXT */:
      return "GL_W_EXT (0x87D8)";
    case 0x87D9 /* GL_NEGATIVE_X_EXT */:
      return "GL_NEGATIVE_X_EXT (0x87D9)";
    case 0x87DA /* GL_NEGATIVE_Y_EXT */:
      return "GL_NEGATIVE_Y_EXT (0x87DA)";
    case 0x87DB /* GL_NEGATIVE_Z_EXT */:
      return "GL_NEGATIVE_Z_EXT (0x87DB)";
    case 0x87DC /* GL_NEGATIVE_W_EXT */:
      return "GL_NEGATIVE_W_EXT (0x87DC)";
    case 0x87DD /* GL_ZERO_EXT */:
      return "GL_ZERO_EXT (0x87DD)";
    case 0x87DE /* GL_ONE_EXT */:
      return "GL_ONE_EXT (0x87DE)";
    case 0x87DF /* GL_NEGATIVE_ONE_EXT */:
      return "GL_NEGATIVE_ONE_EXT (0x87DF)";
    case 0x87E0 /* GL_NORMALIZED_RANGE_EXT */:
      return "GL_NORMALIZED_RANGE_EXT (0x87E0)";
    case 0x87E1 /* GL_FULL_RANGE_EXT */:
      return "GL_FULL_RANGE_EXT (0x87E1)";
    case 0x87E2 /* GL_CURRENT_VERTEX_EXT */:
      return "GL_CURRENT_VERTEX_EXT (0x87E2)";
    case 0x87E3 /* GL_MVP_MATRIX_EXT */:
      return "GL_MVP_MATRIX_EXT (0x87E3)";
    case 0x87E4 /* GL_VARIANT_VALUE_EXT */:
      return "GL_VARIANT_VALUE_EXT (0x87E4)";
    case 0x87E5 /* GL_VARIANT_DATATYPE_EXT */:
      return "GL_VARIANT_DATATYPE_EXT (0x87E5)";
    case 0x87E6 /* GL_VARIANT_ARRAY_STRIDE_EXT */:
      return "GL_VARIANT_ARRAY_STRIDE_EXT (0x87E6)";
    case 0x87E7 /* GL_VARIANT_ARRAY_TYPE_EXT */:
      return "GL_VARIANT_ARRAY_TYPE_EXT (0x87E7)";
    case 0x87E8 /* GL_VARIANT_ARRAY_EXT */:
      return "GL_VARIANT_ARRAY_EXT (0x87E8)";
    case 0x87E9 /* GL_VARIANT_ARRAY_POINTER_EXT */:
      return "GL_VARIANT_ARRAY_POINTER_EXT (0x87E9)";
    case 0x87EA /* GL_INVARIANT_VALUE_EXT */:
      return "GL_INVARIANT_VALUE_EXT (0x87EA)";
    case 0x87EB /* GL_INVARIANT_DATATYPE_EXT */:
      return "GL_INVARIANT_DATATYPE_EXT (0x87EB)";
    case 0x87EC /* GL_LOCAL_CONSTANT_VALUE_EXT */:
      return "GL_LOCAL_CONSTANT_VALUE_EXT (0x87EC)";
    case 0x87ED /* GL_LOCAL_CONSTANT_DATATYPE_EXT */:
      return "GL_LOCAL_CONSTANT_DATATYPE_EXT (0x87ED)";

    /* ------------------------ GL_EXT_vertex_weighting ------------------------
     */

    // case 0x0BA3 /* GL_MODELVIEW0_STACK_DEPTH_EXT */: return
    // "GL_MODELVIEW0_STACK_DEPTH_EXT (0x0BA3)"; case 0x0BA6 /*
    // GL_MODELVIEW0_MATRIX_EXT */: return "GL_MODELVIEW0_MATRIX_EXT (0x0BA6)";
    // case 0x1700 /* GL_MODELVIEW0_EXT */: return "GL_MODELVIEW0_EXT (0x1700)";
    case 0x8502 /* GL_MODELVIEW1_STACK_DEPTH_EXT */:
      return "GL_MODELVIEW1_STACK_DEPTH_EXT (0x8502)";
    case 0x8506 /* GL_MODELVIEW1_MATRIX_EXT */:
      return "GL_MODELVIEW1_MATRIX_EXT (0x8506)";
    case 0x8509 /* GL_VERTEX_WEIGHTING_EXT */:
      return "GL_VERTEX_WEIGHTING_EXT (0x8509)";
    // case 0x850A /* GL_MODELVIEW1_EXT */: return "GL_MODELVIEW1_EXT (0x850A)";
    case 0x850B /* GL_CURRENT_VERTEX_WEIGHT_EXT */:
      return "GL_CURRENT_VERTEX_WEIGHT_EXT (0x850B)";
    case 0x850C /* GL_VERTEX_WEIGHT_ARRAY_EXT */:
      return "GL_VERTEX_WEIGHT_ARRAY_EXT (0x850C)";
    case 0x850D /* GL_VERTEX_WEIGHT_ARRAY_SIZE_EXT */:
      return "GL_VERTEX_WEIGHT_ARRAY_SIZE_EXT (0x850D)";
    case 0x850E /* GL_VERTEX_WEIGHT_ARRAY_TYPE_EXT */:
      return "GL_VERTEX_WEIGHT_ARRAY_TYPE_EXT (0x850E)";
    case 0x850F /* GL_VERTEX_WEIGHT_ARRAY_STRIDE_EXT */:
      return "GL_VERTEX_WEIGHT_ARRAY_STRIDE_EXT (0x850F)";
    case 0x8510 /* GL_VERTEX_WEIGHT_ARRAY_POINTER_EXT */:
      return "GL_VERTEX_WEIGHT_ARRAY_POINTER_EXT (0x8510)";

      /* ------------------------- GL_EXT_x11_sync_object
       * ------------------------ */

    case 0x90E1 /* GL_SYNC_X11_FENCE_EXT */:
      return "GL_SYNC_X11_FENCE_EXT (0x90E1)"; /* ----------------------
                                                  GL_GREMEDY_frame_terminator
                                                  ---------------------- */

      /* ------------------------ GL_GREMEDY_string_marker
       * ----------------------- */

      /* --------------------- GL_HP_convolution_border_modes
       * -------------------- */

      /* ------------------------- GL_HP_image_transform
       * ------------------------- */

      /* -------------------------- GL_HP_occlusion_test
       * ------------------------- */

    case 0x8165 /* GL_OCCLUSION_TEST_HP */:
      return "GL_OCCLUSION_TEST_HP (0x8165)";
    case 0x8166 /* GL_OCCLUSION_TEST_RESULT_HP */:
      return "GL_OCCLUSION_TEST_RESULT_HP (0x8166)";

      /* ------------------------- GL_HP_texture_lighting
       * ------------------------ */

      /* --------------------------- GL_IBM_cull_vertex
       * -------------------------- */
      /* ---------------------- GL_IBM_multimode_draw_arrays
       * --------------------- */

      /* ------------------------- GL_IBM_rasterpos_clip
       * ------------------------- */
      /* --------------------------- GL_IBM_static_data
       * -------------------------- */

      /* --------------------- GL_IBM_texture_mirrored_repeat
       * -------------------- */

      // case 0x8370 /* GL_MIRRORED_REPEAT_IBM */: return
      // "GL_MIRRORED_REPEAT_IBM (0x8370)";

      /* ----------------------- GL_IBM_vertex_array_lists
       * ----------------------- */

      /* -------------------------- GL_INGR_color_clamp
       * -------------------------- */

    case 0x8560 /* GL_RED_MIN_CLAMP_INGR */:
      return "GL_RED_MIN_CLAMP_INGR (0x8560)";
    case 0x8561 /* GL_GREEN_MIN_CLAMP_INGR */:
      return "GL_GREEN_MIN_CLAMP_INGR (0x8561)";
    case 0x8562 /* GL_BLUE_MIN_CLAMP_INGR */:
      return "GL_BLUE_MIN_CLAMP_INGR (0x8562)";
    case 0x8563 /* GL_ALPHA_MIN_CLAMP_INGR */:
      return "GL_ALPHA_MIN_CLAMP_INGR (0x8563)";
    case 0x8564 /* GL_RED_MAX_CLAMP_INGR */:
      return "GL_RED_MAX_CLAMP_INGR (0x8564)";
    case 0x8565 /* GL_GREEN_MAX_CLAMP_INGR */:
      return "GL_GREEN_MAX_CLAMP_INGR (0x8565)";
    case 0x8566 /* GL_BLUE_MAX_CLAMP_INGR */:
      return "GL_BLUE_MAX_CLAMP_INGR (0x8566)";
    case 0x8567 /* GL_ALPHA_MAX_CLAMP_INGR */:
      return "GL_ALPHA_MAX_CLAMP_INGR (0x8567)";

      /* ------------------------- GL_INGR_interlace_read
       * ------------------------ */

    case 0x8568 /* GL_INTERLACE_READ_INGR */:
      return "GL_INTERLACE_READ_INGR (0x8568)";

      /* ------------------------ GL_INTEL_parallel_arrays
       * ----------------------- */

    case 0x83F4 /* GL_PARALLEL_ARRAYS_INTEL */:
      return "GL_PARALLEL_ARRAYS_INTEL (0x83F4)";
    case 0x83F5 /* GL_VERTEX_ARRAY_PARALLEL_POINTERS_INTEL */:
      return "GL_VERTEX_ARRAY_PARALLEL_POINTERS_INTEL (0x83F5)";
    case 0x83F6 /* GL_NORMAL_ARRAY_PARALLEL_POINTERS_INTEL */:
      return "GL_NORMAL_ARRAY_PARALLEL_POINTERS_INTEL (0x83F6)";
    case 0x83F7 /* GL_COLOR_ARRAY_PARALLEL_POINTERS_INTEL */:
      return "GL_COLOR_ARRAY_PARALLEL_POINTERS_INTEL (0x83F7)";
    case 0x83F8 /* GL_TEXTURE_COORD_ARRAY_PARALLEL_POINTERS_INTEL */:
      return "GL_TEXTURE_COORD_ARRAY_PARALLEL_POINTERS_INTEL (0x83F8)";

    /* ------------------------ GL_INTEL_texture_scissor -----------------------
     */

    /* ------------------------------ GL_KHR_debug -----------------------------
     */

    // case 0x00000002 /* GL_CONTEXT_FLAG_DEBUG_BIT */: return
    // "GL_CONTEXT_FLAG_DEBUG_BIT (0x00000002)"; case 0x0503 /* GL_STACK_OVERFLOW
    // */: return "GL_STACK_OVERFLOW (0x0503)"; case 0x0504 /* GL_STACK_UNDERFLOW
    // */: return "GL_STACK_UNDERFLOW (0x0504)"; case 0x8242 /*
    // GL_DEBUG_OUTPUT_SYNCHRONOUS */: return "GL_DEBUG_OUTPUT_SYNCHRONOUS
    // (0x8242)"; case 0x8243 /* GL_DEBUG_NEXT_LOGGED_MESSAGE_LENGTH */: return
    // "GL_DEBUG_NEXT_LOGGED_MESSAGE_LENGTH (0x8243)"; case 0x8244 /*
    // GL_DEBUG_CALLBACK_FUNCTION */: return "GL_DEBUG_CALLBACK_FUNCTION
    // (0x8244)"; case 0x8245 /* GL_DEBUG_CALLBACK_USER_PARAM */: return
    // "GL_DEBUG_CALLBACK_USER_PARAM (0x8245)"; case 0x8246 /*
    // GL_DEBUG_SOURCE_API */: return "GL_DEBUG_SOURCE_API (0x8246)"; case 0x8247
    // /* GL_DEBUG_SOURCE_WINDOW_SYSTEM */: return "GL_DEBUG_SOURCE_WINDOW_SYSTEM
    // (0x8247)"; case 0x8248 /* GL_DEBUG_SOURCE_SHADER_COMPILER */: return
    // "GL_DEBUG_SOURCE_SHADER_COMPILER (0x8248)"; case 0x8249 /*
    // GL_DEBUG_SOURCE_THIRD_PARTY */: return "GL_DEBUG_SOURCE_THIRD_PARTY
    // (0x8249)"; case 0x824A /* GL_DEBUG_SOURCE_APPLICATION */: return
    // "GL_DEBUG_SOURCE_APPLICATION (0x824A)"; case 0x824B /*
    // GL_DEBUG_SOURCE_OTHER */: return "GL_DEBUG_SOURCE_OTHER (0x824B)"; case
    // 0x824C /* GL_DEBUG_TYPE_ERROR */: return "GL_DEBUG_TYPE_ERROR (0x824C)";
    // case 0x824D /* GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR */: return
    // "GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR (0x824D)"; case 0x824E /*
    // GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR */: return
    // "GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR (0x824E)"; case 0x824F /*
    // GL_DEBUG_TYPE_PORTABILITY */: return "GL_DEBUG_TYPE_PORTABILITY (0x824F)";
    // case 0x8250 /* GL_DEBUG_TYPE_PERFORMANCE */: return
    // "GL_DEBUG_TYPE_PERFORMANCE (0x8250)"; case 0x8251 /* GL_DEBUG_TYPE_OTHER
    // */: return "GL_DEBUG_TYPE_OTHER (0x8251)";
    case 0x8268 /* GL_DEBUG_TYPE_MARKER */:
      return "GL_DEBUG_TYPE_MARKER (0x8268)";
    case 0x8269 /* GL_DEBUG_TYPE_PUSH_GROUP */:
      return "GL_DEBUG_TYPE_PUSH_GROUP (0x8269)";
    case 0x826A /* GL_DEBUG_TYPE_POP_GROUP */:
      return "GL_DEBUG_TYPE_POP_GROUP (0x826A)";
    case 0x826B /* GL_DEBUG_SEVERITY_NOTIFICATION */:
      return "GL_DEBUG_SEVERITY_NOTIFICATION (0x826B)";
    case 0x826C /* GL_MAX_DEBUG_GROUP_STACK_DEPTH */:
      return "GL_MAX_DEBUG_GROUP_STACK_DEPTH (0x826C)";
    case 0x826D /* GL_DEBUG_GROUP_STACK_DEPTH */:
      return "GL_DEBUG_GROUP_STACK_DEPTH (0x826D)";
    case 0x82E0 /* GL_BUFFER */:
      return "GL_BUFFER (0x82E0)";
    case 0x82E1 /* GL_SHADER */:
      return "GL_SHADER (0x82E1)";
    case 0x82E2 /* GL_PROGRAM */:
      return "GL_PROGRAM (0x82E2)";
    case 0x82E3 /* GL_QUERY */:
      return "GL_QUERY (0x82E3)";
    case 0x82E4 /* GL_PROGRAM_PIPELINE */:
      return "GL_PROGRAM_PIPELINE (0x82E4)";
    case 0x82E6 /* GL_SAMPLER */:
      return "GL_SAMPLER (0x82E6)";
    case 0x82E7 /* GL_DISPLAY_LIST */:
      return "GL_DISPLAY_LIST (0x82E7)";
    case 0x82E8 /* GL_MAX_LABEL_LENGTH */:
      return "GL_MAX_LABEL_LENGTH (0x82E8)";
    // case 0x9143 /* GL_MAX_DEBUG_MESSAGE_LENGTH */: return
    // "GL_MAX_DEBUG_MESSAGE_LENGTH (0x9143)"; case 0x9144 /*
    // GL_MAX_DEBUG_LOGGED_MESSAGES */: return "GL_MAX_DEBUG_LOGGED_MESSAGES
    // (0x9144)"; case 0x9145 /* GL_DEBUG_LOGGED_MESSAGES */: return
    // "GL_DEBUG_LOGGED_MESSAGES (0x9145)"; case 0x9146 /* GL_DEBUG_SEVERITY_HIGH
    // */: return "GL_DEBUG_SEVERITY_HIGH (0x9146)"; case 0x9147 /*
    // GL_DEBUG_SEVERITY_MEDIUM */: return "GL_DEBUG_SEVERITY_MEDIUM (0x9147)";
    // case 0x9148 /* GL_DEBUG_SEVERITY_LOW */: return "GL_DEBUG_SEVERITY_LOW
    // (0x9148)";
    case 0x92E0 /* GL_DEBUG_OUTPUT */:
      return "GL_DEBUG_OUTPUT (0x92E0)"; /* ------------------
                                            GL_KHR_texture_compression_astc_ldr
                                            ------------------ */

    case 0x93B0 /* GL_COMPRESSED_RGBA_ASTC_4x4_KHR */:
      return "GL_COMPRESSED_RGBA_ASTC_4x4_KHR (0x93B0)";
    case 0x93B1 /* GL_COMPRESSED_RGBA_ASTC_5x4_KHR */:
      return "GL_COMPRESSED_RGBA_ASTC_5x4_KHR (0x93B1)";
    case 0x93B2 /* GL_COMPRESSED_RGBA_ASTC_5x5_KHR */:
      return "GL_COMPRESSED_RGBA_ASTC_5x5_KHR (0x93B2)";
    case 0x93B3 /* GL_COMPRESSED_RGBA_ASTC_6x5_KHR */:
      return "GL_COMPRESSED_RGBA_ASTC_6x5_KHR (0x93B3)";
    case 0x93B4 /* GL_COMPRESSED_RGBA_ASTC_6x6_KHR */:
      return "GL_COMPRESSED_RGBA_ASTC_6x6_KHR (0x93B4)";
    case 0x93B5 /* GL_COMPRESSED_RGBA_ASTC_8x5_KHR */:
      return "GL_COMPRESSED_RGBA_ASTC_8x5_KHR (0x93B5)";
    case 0x93B6 /* GL_COMPRESSED_RGBA_ASTC_8x6_KHR */:
      return "GL_COMPRESSED_RGBA_ASTC_8x6_KHR (0x93B6)";
    case 0x93B7 /* GL_COMPRESSED_RGBA_ASTC_8x8_KHR */:
      return "GL_COMPRESSED_RGBA_ASTC_8x8_KHR (0x93B7)";
    case 0x93B8 /* GL_COMPRESSED_RGBA_ASTC_10x5_KHR */:
      return "GL_COMPRESSED_RGBA_ASTC_10x5_KHR (0x93B8)";
    case 0x93B9 /* GL_COMPRESSED_RGBA_ASTC_10x6_KHR */:
      return "GL_COMPRESSED_RGBA_ASTC_10x6_KHR (0x93B9)";
    case 0x93BA /* GL_COMPRESSED_RGBA_ASTC_10x8_KHR */:
      return "GL_COMPRESSED_RGBA_ASTC_10x8_KHR (0x93BA)";
    case 0x93BB /* GL_COMPRESSED_RGBA_ASTC_10x10_KHR */:
      return "GL_COMPRESSED_RGBA_ASTC_10x10_KHR (0x93BB)";
    case 0x93BC /* GL_COMPRESSED_RGBA_ASTC_12x10_KHR */:
      return "GL_COMPRESSED_RGBA_ASTC_12x10_KHR (0x93BC)";
    case 0x93BD /* GL_COMPRESSED_RGBA_ASTC_12x12_KHR */:
      return "GL_COMPRESSED_RGBA_ASTC_12x12_KHR (0x93BD)";
    case 0x93D0 /* GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR */:
      return "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR (0x93D0)";
    case 0x93D1 /* GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR */:
      return "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR (0x93D1)";
    case 0x93D2 /* GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR */:
      return "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR (0x93D2)";
    case 0x93D3 /* GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR */:
      return "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR (0x93D3)";
    case 0x93D4 /* GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR */:
      return "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR (0x93D4)";
    case 0x93D5 /* GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR */:
      return "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR (0x93D5)";
    case 0x93D6 /* GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR */:
      return "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR (0x93D6)";
    case 0x93D7 /* GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR */:
      return "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR (0x93D7)";
    case 0x93D8 /* GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR */:
      return "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR (0x93D8)";
    case 0x93D9 /* GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR */:
      return "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR (0x93D9)";
    case 0x93DA /* GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR */:
      return "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR (0x93DA)";
    case 0x93DB /* GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR */:
      return "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR (0x93DB)";
    case 0x93DC /* GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR */:
      return "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR (0x93DC)";
    case 0x93DD /* GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR */:
      return "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR (0x93DD)";

      /* -------------------------- GL_KTX_buffer_region
       * ------------------------- */

      // case 0x0 /* GL_KTX_FRONT_REGION */: return "GL_KTX_FRONT_REGION (0x0)";
      // case 0x1 /* GL_KTX_BACK_REGION */: return "GL_KTX_BACK_REGION (0x1)";
      // case 0x2 /* GL_KTX_Z_REGION */: return "GL_KTX_Z_REGION (0x2)";
      // case 0x3 /* GL_KTX_STENCIL_REGION */: return "GL_KTX_STENCIL_REGION
      // (0x3)";

      /* ------------------------- GL_MESAX_texture_stack
       * ------------------------ */

    case 0x8759 /* GL_TEXTURE_1D_STACK_MESAX */:
      return "GL_TEXTURE_1D_STACK_MESAX (0x8759)";
    case 0x875A /* GL_TEXTURE_2D_STACK_MESAX */:
      return "GL_TEXTURE_2D_STACK_MESAX (0x875A)";
    case 0x875B /* GL_PROXY_TEXTURE_1D_STACK_MESAX */:
      return "GL_PROXY_TEXTURE_1D_STACK_MESAX (0x875B)";
    case 0x875C /* GL_PROXY_TEXTURE_2D_STACK_MESAX */:
      return "GL_PROXY_TEXTURE_2D_STACK_MESAX (0x875C)";
    case 0x875D /* GL_TEXTURE_1D_STACK_BINDING_MESAX */:
      return "GL_TEXTURE_1D_STACK_BINDING_MESAX (0x875D)";
    case 0x875E /* GL_TEXTURE_2D_STACK_BINDING_MESAX */:
      return "GL_TEXTURE_2D_STACK_BINDING_MESAX (0x875E)";

      /* -------------------------- GL_MESA_pack_invert
       * -------------------------- */

    case 0x8758 /* GL_PACK_INVERT_MESA */:
      return "GL_PACK_INVERT_MESA (0x8758)";

    /* ------------------------- GL_MESA_resize_buffers ------------------------
     */

    /* --------------------------- GL_MESA_window_pos --------------------------
     */

    /* ------------------------- GL_MESA_ycbcr_texture -------------------------
     */

    // case 0x85BA /* GL_UNSIGNED_SHORT_8_8_MESA */: return
    // "GL_UNSIGNED_SHORT_8_8_MESA (0x85BA)"; case 0x85BB /*
    // GL_UNSIGNED_SHORT_8_8_REV_MESA */: return "GL_UNSIGNED_SHORT_8_8_REV_MESA
    // (0x85BB)";
    case 0x8757 /* GL_YCBCR_MESA */:
      return "GL_YCBCR_MESA (0x8757)";

      /* ------------------------- GL_NVX_gpu_memory_info
       * ------------------------ */

    case 0x9047 /* GL_GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX */:
      return "GL_GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX (0x9047)";
    case 0x9048 /* GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX */:
      return "GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX (0x9048)";
    case 0x9049 /* GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX */:
      return "GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX (0x9049)";
    case 0x904A /* GL_GPU_MEMORY_INFO_EVICTION_COUNT_NVX */:
      return "GL_GPU_MEMORY_INFO_EVICTION_COUNT_NVX (0x904A)";
    case 0x904B /* GL_GPU_MEMORY_INFO_EVICTED_MEMORY_NVX */:
      return "GL_GPU_MEMORY_INFO_EVICTED_MEMORY_NVX (0x904B)";

      /* ------------------------- GL_NV_bindless_texture
       * ------------------------ */

      /* --------------------------- GL_NV_blend_square
       * -------------------------- */

      /* ------------------------ GL_NV_conditional_render
       * ----------------------- */

      // case 0x8E13 /* GL_QUERY_WAIT_NV */: return "GL_QUERY_WAIT_NV (0x8E13)";
      // case 0x8E14 /* GL_QUERY_NO_WAIT_NV */: return "GL_QUERY_NO_WAIT_NV
      // (0x8E14)"; case 0x8E15 /* GL_QUERY_BY_REGION_WAIT_NV */: return
      // "GL_QUERY_BY_REGION_WAIT_NV (0x8E15)"; case 0x8E16 /*
      // GL_QUERY_BY_REGION_NO_WAIT_NV */: return "GL_QUERY_BY_REGION_NO_WAIT_NV
      // (0x8E16)";

      /* ----------------------- GL_NV_copy_depth_to_color
       * ----------------------- */

    case 0x886E /* GL_DEPTH_STENCIL_TO_RGBA_NV */:
      return "GL_DEPTH_STENCIL_TO_RGBA_NV (0x886E)";
    case 0x886F /* GL_DEPTH_STENCIL_TO_BGRA_NV */:
      return "GL_DEPTH_STENCIL_TO_BGRA_NV (0x886F)";

      /* ---------------------------- GL_NV_copy_image
       * --------------------------- */

      /* ------------------------ GL_NV_depth_buffer_float
       * ----------------------- */

    case 0x8DAB /* GL_DEPTH_COMPONENT32F_NV */:
      return "GL_DEPTH_COMPONENT32F_NV (0x8DAB)";
    case 0x8DAC /* GL_DEPTH32F_STENCIL8_NV */:
      return "GL_DEPTH32F_STENCIL8_NV (0x8DAC)";
    // case 0x8DAD /* GL_FLOAT_32_UNSIGNED_INT_24_8_REV_NV */: return
    // "GL_FLOAT_32_UNSIGNED_INT_24_8_REV_NV (0x8DAD)";
    case 0x8DAF /* GL_DEPTH_BUFFER_FLOAT_MODE_NV */:
      return "GL_DEPTH_BUFFER_FLOAT_MODE_NV (0x8DAF)";

      /* --------------------------- GL_NV_depth_clamp
       * --------------------------- */

      // case 0x864F /* GL_DEPTH_CLAMP_NV */: return "GL_DEPTH_CLAMP_NV
      // (0x864F)";

      /* ---------------------- GL_NV_depth_range_unclamped
       * ---------------------- */

      // case 0x8864 /* GL_SAMPLE_COUNT_BITS_NV */: return
      // "GL_SAMPLE_COUNT_BITS_NV (0x8864)"; case 0x8865 /*
      // GL_CURRENT_SAMPLE_COUNT_QUERY_NV */: return
      // "GL_CURRENT_SAMPLE_COUNT_QUERY_NV (0x8865)"; case 0x8866 /*
      // GL_QUERY_RESULT_NV */: return "GL_QUERY_RESULT_NV (0x8866)"; case 0x8867
      // /* GL_QUERY_RESULT_AVAILABLE_NV */: return "GL_QUERY_RESULT_AVAILABLE_NV
      // (0x8867)"; case 0x8914 /* GL_SAMPLE_COUNT_NV */: return
      // "GL_SAMPLE_COUNT_NV (0x8914)";

      /* ---------------------------- GL_NV_evaluators
       * --------------------------- */

    case 0x86C0 /* GL_EVAL_2D_NV */:
      return "GL_EVAL_2D_NV (0x86C0)";
    case 0x86C1 /* GL_EVAL_TRIANGULAR_2D_NV */:
      return "GL_EVAL_TRIANGULAR_2D_NV (0x86C1)";
    case 0x86C2 /* GL_MAP_TESSELLATION_NV */:
      return "GL_MAP_TESSELLATION_NV (0x86C2)";
    case 0x86C3 /* GL_MAP_ATTRIB_U_ORDER_NV */:
      return "GL_MAP_ATTRIB_U_ORDER_NV (0x86C3)";
    case 0x86C4 /* GL_MAP_ATTRIB_V_ORDER_NV */:
      return "GL_MAP_ATTRIB_V_ORDER_NV (0x86C4)";
    case 0x86C5 /* GL_EVAL_FRACTIONAL_TESSELLATION_NV */:
      return "GL_EVAL_FRACTIONAL_TESSELLATION_NV (0x86C5)";
    case 0x86C6 /* GL_EVAL_VERTEX_ATTRIB0_NV */:
      return "GL_EVAL_VERTEX_ATTRIB0_NV (0x86C6)";
    case 0x86C7 /* GL_EVAL_VERTEX_ATTRIB1_NV */:
      return "GL_EVAL_VERTEX_ATTRIB1_NV (0x86C7)";
    case 0x86C8 /* GL_EVAL_VERTEX_ATTRIB2_NV */:
      return "GL_EVAL_VERTEX_ATTRIB2_NV (0x86C8)";
    case 0x86C9 /* GL_EVAL_VERTEX_ATTRIB3_NV */:
      return "GL_EVAL_VERTEX_ATTRIB3_NV (0x86C9)";
    case 0x86CA /* GL_EVAL_VERTEX_ATTRIB4_NV */:
      return "GL_EVAL_VERTEX_ATTRIB4_NV (0x86CA)";
    case 0x86CB /* GL_EVAL_VERTEX_ATTRIB5_NV */:
      return "GL_EVAL_VERTEX_ATTRIB5_NV (0x86CB)";
    case 0x86CC /* GL_EVAL_VERTEX_ATTRIB6_NV */:
      return "GL_EVAL_VERTEX_ATTRIB6_NV (0x86CC)";
    case 0x86CD /* GL_EVAL_VERTEX_ATTRIB7_NV */:
      return "GL_EVAL_VERTEX_ATTRIB7_NV (0x86CD)";
    case 0x86CE /* GL_EVAL_VERTEX_ATTRIB8_NV */:
      return "GL_EVAL_VERTEX_ATTRIB8_NV (0x86CE)";
    case 0x86CF /* GL_EVAL_VERTEX_ATTRIB9_NV */:
      return "GL_EVAL_VERTEX_ATTRIB9_NV (0x86CF)";
    case 0x86D0 /* GL_EVAL_VERTEX_ATTRIB10_NV */:
      return "GL_EVAL_VERTEX_ATTRIB10_NV (0x86D0)";
    case 0x86D1 /* GL_EVAL_VERTEX_ATTRIB11_NV */:
      return "GL_EVAL_VERTEX_ATTRIB11_NV (0x86D1)";
    case 0x86D2 /* GL_EVAL_VERTEX_ATTRIB12_NV */:
      return "GL_EVAL_VERTEX_ATTRIB12_NV (0x86D2)";
    case 0x86D3 /* GL_EVAL_VERTEX_ATTRIB13_NV */:
      return "GL_EVAL_VERTEX_ATTRIB13_NV (0x86D3)";
    case 0x86D4 /* GL_EVAL_VERTEX_ATTRIB14_NV */:
      return "GL_EVAL_VERTEX_ATTRIB14_NV (0x86D4)";
    case 0x86D5 /* GL_EVAL_VERTEX_ATTRIB15_NV */:
      return "GL_EVAL_VERTEX_ATTRIB15_NV (0x86D5)";
    case 0x86D6 /* GL_MAX_MAP_TESSELLATION_NV */:
      return "GL_MAX_MAP_TESSELLATION_NV (0x86D6)";
    case 0x86D7 /* GL_MAX_RATIONAL_EVAL_ORDER_NV */:
      return "GL_MAX_RATIONAL_EVAL_ORDER_NV (0x86D7)";

    /* ----------------------- GL_NV_explicit_multisample ----------------------
     */

    // case 0x8E50 /* GL_SAMPLE_POSITION_NV */: return "GL_SAMPLE_POSITION_NV
    // (0x8E50)"; case 0x8E51 /* GL_SAMPLE_MASK_NV */: return "GL_SAMPLE_MASK_NV
    // (0x8E51)"; case 0x8E52 /* GL_SAMPLE_MASK_VALUE_NV */: return
    // "GL_SAMPLE_MASK_VALUE_NV (0x8E52)";
    case 0x8E53 /* GL_TEXTURE_BINDING_RENDERBUFFER_NV */:
      return "GL_TEXTURE_BINDING_RENDERBUFFER_NV (0x8E53)";
    case 0x8E54 /* GL_TEXTURE_RENDERBUFFER_DATA_STORE_BINDING_NV */:
      return "GL_TEXTURE_RENDERBUFFER_DATA_STORE_BINDING_NV (0x8E54)";
    case 0x8E55 /* GL_TEXTURE_RENDERBUFFER_NV */:
      return "GL_TEXTURE_RENDERBUFFER_NV (0x8E55)";
    case 0x8E56 /* GL_SAMPLER_RENDERBUFFER_NV */:
      return "GL_SAMPLER_RENDERBUFFER_NV (0x8E56)";
    case 0x8E57 /* GL_INT_SAMPLER_RENDERBUFFER_NV */:
      return "GL_INT_SAMPLER_RENDERBUFFER_NV (0x8E57)";
    case 0x8E58 /* GL_UNSIGNED_INT_SAMPLER_RENDERBUFFER_NV */:
      return "GL_UNSIGNED_INT_SAMPLER_RENDERBUFFER_NV (0x8E58)";
      // case 0x8E59 /* GL_MAX_SAMPLE_MASK_WORDS_NV */: return
      // "GL_MAX_SAMPLE_MASK_WORDS_NV (0x8E59)";

      /* ------------------------------ GL_NV_fence
       * ------------------------------ */

    case 0x84F2 /* GL_ALL_COMPLETED_NV */:
      return "GL_ALL_COMPLETED_NV (0x84F2)";
    case 0x84F3 /* GL_FENCE_STATUS_NV */:
      return "GL_FENCE_STATUS_NV (0x84F3)";
    case 0x84F4 /* GL_FENCE_CONDITION_NV */:
      return "GL_FENCE_CONDITION_NV (0x84F4)";

      /* --------------------------- GL_NV_float_buffer
       * -------------------------- */

    case 0x8880 /* GL_FLOAT_R_NV */:
      return "GL_FLOAT_R_NV (0x8880)";
    case 0x8881 /* GL_FLOAT_RG_NV */:
      return "GL_FLOAT_RG_NV (0x8881)";
    case 0x8882 /* GL_FLOAT_RGB_NV */:
      return "GL_FLOAT_RGB_NV (0x8882)";
    case 0x8883 /* GL_FLOAT_RGBA_NV */:
      return "GL_FLOAT_RGBA_NV (0x8883)";
    case 0x8884 /* GL_FLOAT_R16_NV */:
      return "GL_FLOAT_R16_NV (0x8884)";
    case 0x8885 /* GL_FLOAT_R32_NV */:
      return "GL_FLOAT_R32_NV (0x8885)";
    case 0x8886 /* GL_FLOAT_RG16_NV */:
      return "GL_FLOAT_RG16_NV (0x8886)";
    case 0x8887 /* GL_FLOAT_RG32_NV */:
      return "GL_FLOAT_RG32_NV (0x8887)";
    case 0x8888 /* GL_FLOAT_RGB16_NV */:
      return "GL_FLOAT_RGB16_NV (0x8888)";
    case 0x8889 /* GL_FLOAT_RGB32_NV */:
      return "GL_FLOAT_RGB32_NV (0x8889)";
    case 0x888A /* GL_FLOAT_RGBA16_NV */:
      return "GL_FLOAT_RGBA16_NV (0x888A)";
    case 0x888B /* GL_FLOAT_RGBA32_NV */:
      return "GL_FLOAT_RGBA32_NV (0x888B)";
    case 0x888C /* GL_TEXTURE_FLOAT_COMPONENTS_NV */:
      return "GL_TEXTURE_FLOAT_COMPONENTS_NV (0x888C)";
    case 0x888D /* GL_FLOAT_CLEAR_COLOR_VALUE_NV */:
      return "GL_FLOAT_CLEAR_COLOR_VALUE_NV (0x888D)";
    case 0x888E /* GL_FLOAT_RGBA_MODE_NV */:
      return "GL_FLOAT_RGBA_MODE_NV (0x888E)";

      /* --------------------------- GL_NV_fog_distance
       * -------------------------- */

    case 0x855A /* GL_FOG_DISTANCE_MODE_NV */:
      return "GL_FOG_DISTANCE_MODE_NV (0x855A)";
    case 0x855B /* GL_EYE_RADIAL_NV */:
      return "GL_EYE_RADIAL_NV (0x855B)";
    case 0x855C /* GL_EYE_PLANE_ABSOLUTE_NV */:
      return "GL_EYE_PLANE_ABSOLUTE_NV (0x855C)";

      /* ------------------------- GL_NV_fragment_program
       * ------------------------ */

    case 0x8868 /* GL_MAX_FRAGMENT_PROGRAM_LOCAL_PARAMETERS_NV */:
      return "GL_MAX_FRAGMENT_PROGRAM_LOCAL_PARAMETERS_NV (0x8868)";
    case 0x8870 /* GL_FRAGMENT_PROGRAM_NV */:
      return "GL_FRAGMENT_PROGRAM_NV (0x8870)";
    // case 0x8871 /* GL_MAX_TEXTURE_COORDS_NV */: return
    // "GL_MAX_TEXTURE_COORDS_NV (0x8871)"; case 0x8872 /*
    // GL_MAX_TEXTURE_IMAGE_UNITS_NV */: return "GL_MAX_TEXTURE_IMAGE_UNITS_NV
    // (0x8872)";
    case 0x8873 /* GL_FRAGMENT_PROGRAM_BINDING_NV */:
      return "GL_FRAGMENT_PROGRAM_BINDING_NV (0x8873)";
      // case 0x8874 /* GL_PROGRAM_ERROR_STRING_NV */: return
      // "GL_PROGRAM_ERROR_STRING_NV (0x8874)";/* ------------------------
      // GL_NV_fragment_program2 ------------------------ */

    case 0x88F5 /* GL_MAX_PROGRAM_CALL_DEPTH_NV */:
      return "GL_MAX_PROGRAM_CALL_DEPTH_NV (0x88F5)";
    case 0x88F6 /* GL_MAX_PROGRAM_IF_DEPTH_NV */:
      return "GL_MAX_PROGRAM_IF_DEPTH_NV (0x88F6)";
    case 0x88F7 /* GL_MAX_PROGRAM_LOOP_DEPTH_NV */:
      return "GL_MAX_PROGRAM_LOOP_DEPTH_NV (0x88F7)";
    case 0x88F8 /* GL_MAX_PROGRAM_LOOP_COUNT_NV */:
      return "GL_MAX_PROGRAM_LOOP_COUNT_NV (0x88F8)";

    /* ------------------------ GL_NV_fragment_program4 ------------------------
     */

    /* --------------------- GL_NV_fragment_program_option ---------------------
     */

    /* ----------------- GL_NV_framebuffer_multisample_coverage ----------------
     */

    // case 0x8CAB /* GL_RENDERBUFFER_COVERAGE_SAMPLES_NV */: return
    // "GL_RENDERBUFFER_COVERAGE_SAMPLES_NV (0x8CAB)";
    case 0x8E10 /* GL_RENDERBUFFER_COLOR_SAMPLES_NV */:
      return "GL_RENDERBUFFER_COLOR_SAMPLES_NV (0x8E10)";
    case 0x8E11 /* GL_MAX_MULTISAMPLE_COVERAGE_MODES_NV */:
      return "GL_MAX_MULTISAMPLE_COVERAGE_MODES_NV (0x8E11)";
    case 0x8E12 /* GL_MULTISAMPLE_COVERAGE_MODES_NV */:
      return "GL_MULTISAMPLE_COVERAGE_MODES_NV (0x8E12)"; /* ------------------------
                                                             GL_NV_geometry_program4
                                                             ------------------------
                                                           */

    case 0x8C26 /* GL_GEOMETRY_PROGRAM_NV */:
      return "GL_GEOMETRY_PROGRAM_NV (0x8C26)";
    case 0x8C27 /* GL_MAX_PROGRAM_OUTPUT_VERTICES_NV */:
      return "GL_MAX_PROGRAM_OUTPUT_VERTICES_NV (0x8C27)";
    case 0x8C28 /* GL_MAX_PROGRAM_TOTAL_OUTPUT_COMPONENTS_NV */:
      return "GL_MAX_PROGRAM_TOTAL_OUTPUT_COMPONENTS_NV (0x8C28)"; /* -------------------------
                                                                      GL_NV_geometry_shader4
                                                                      ------------------------
                                                                    */

    /* --------------------------- GL_NV_gpu_program4 --------------------------
     */

    // case 0x8904 /* GL_MIN_PROGRAM_TEXEL_OFFSET_NV */: return
    // "GL_MIN_PROGRAM_TEXEL_OFFSET_NV (0x8904)"; case 0x8905 /*
    // GL_MAX_PROGRAM_TEXEL_OFFSET_NV */: return "GL_MAX_PROGRAM_TEXEL_OFFSET_NV
    // (0x8905)";
    case 0x8906 /* GL_PROGRAM_ATTRIB_COMPONENTS_NV */:
      return "GL_PROGRAM_ATTRIB_COMPONENTS_NV (0x8906)";
    case 0x8907 /* GL_PROGRAM_RESULT_COMPONENTS_NV */:
      return "GL_PROGRAM_RESULT_COMPONENTS_NV (0x8907)";
    case 0x8908 /* GL_MAX_PROGRAM_ATTRIB_COMPONENTS_NV */:
      return "GL_MAX_PROGRAM_ATTRIB_COMPONENTS_NV (0x8908)";
    case 0x8909 /* GL_MAX_PROGRAM_RESULT_COMPONENTS_NV */:
      return "GL_MAX_PROGRAM_RESULT_COMPONENTS_NV (0x8909)";
    case 0x8DA5 /* GL_MAX_PROGRAM_GENERIC_ATTRIBS_NV */:
      return "GL_MAX_PROGRAM_GENERIC_ATTRIBS_NV (0x8DA5)";
    case 0x8DA6 /* GL_MAX_PROGRAM_GENERIC_RESULTS_NV */:
      return "GL_MAX_PROGRAM_GENERIC_RESULTS_NV (0x8DA6)";

      /* --------------------------- GL_NV_gpu_program5
       * -------------------------- */

      // case 0x8E5A /* GL_MAX_GEOMETRY_PROGRAM_INVOCATIONS_NV */: return
      // "GL_MAX_GEOMETRY_PROGRAM_INVOCATIONS_NV (0x8E5A)"; case 0x8E5B /*
      // GL_MIN_FRAGMENT_INTERPOLATION_OFFSET_NV */: return
      // "GL_MIN_FRAGMENT_INTERPOLATION_OFFSET_NV (0x8E5B)"; case 0x8E5C /*
      // GL_MAX_FRAGMENT_INTERPOLATION_OFFSET_NV */: return
      // "GL_MAX_FRAGMENT_INTERPOLATION_OFFSET_NV (0x8E5C)"; case 0x8E5D /*
      // GL_FRAGMENT_PROGRAM_INTERPOLATION_OFFSET_BITS_NV */: return
      // "GL_FRAGMENT_PROGRAM_INTERPOLATION_OFFSET_BITS_NV (0x8E5D)"; case 0x8E5E
      // /* GL_MIN_PROGRAM_TEXTURE_GATHER_OFFSET_NV */: return
      // "GL_MIN_PROGRAM_TEXTURE_GATHER_OFFSET_NV (0x8E5E)"; case 0x8E5F /*
      // GL_MAX_PROGRAM_TEXTURE_GATHER_OFFSET_NV */: return
      // "GL_MAX_PROGRAM_TEXTURE_GATHER_OFFSET_NV (0x8E5F)";

      /* ------------------------- GL_NV_gpu_program_fp64
       * ------------------------ */

      /* --------------------------- GL_NV_gpu_shader5
       * --------------------------- */

    case 0x140E /* GL_INT64_NV */:
      return "GL_INT64_NV (0x140E)";
    case 0x140F /* GL_UNSIGNED_INT64_NV */:
      return "GL_UNSIGNED_INT64_NV (0x140F)";
    case 0x8FE0 /* GL_INT8_NV */:
      return "GL_INT8_NV (0x8FE0)";
    case 0x8FE1 /* GL_INT8_VEC2_NV */:
      return "GL_INT8_VEC2_NV (0x8FE1)";
    case 0x8FE2 /* GL_INT8_VEC3_NV */:
      return "GL_INT8_VEC3_NV (0x8FE2)";
    case 0x8FE3 /* GL_INT8_VEC4_NV */:
      return "GL_INT8_VEC4_NV (0x8FE3)";
    case 0x8FE4 /* GL_INT16_NV */:
      return "GL_INT16_NV (0x8FE4)";
    case 0x8FE5 /* GL_INT16_VEC2_NV */:
      return "GL_INT16_VEC2_NV (0x8FE5)";
    case 0x8FE6 /* GL_INT16_VEC3_NV */:
      return "GL_INT16_VEC3_NV (0x8FE6)";
    case 0x8FE7 /* GL_INT16_VEC4_NV */:
      return "GL_INT16_VEC4_NV (0x8FE7)";
    case 0x8FE9 /* GL_INT64_VEC2_NV */:
      return "GL_INT64_VEC2_NV (0x8FE9)";
    case 0x8FEA /* GL_INT64_VEC3_NV */:
      return "GL_INT64_VEC3_NV (0x8FEA)";
    case 0x8FEB /* GL_INT64_VEC4_NV */:
      return "GL_INT64_VEC4_NV (0x8FEB)";
    case 0x8FEC /* GL_UNSIGNED_INT8_NV */:
      return "GL_UNSIGNED_INT8_NV (0x8FEC)";
    case 0x8FED /* GL_UNSIGNED_INT8_VEC2_NV */:
      return "GL_UNSIGNED_INT8_VEC2_NV (0x8FED)";
    case 0x8FEE /* GL_UNSIGNED_INT8_VEC3_NV */:
      return "GL_UNSIGNED_INT8_VEC3_NV (0x8FEE)";
    case 0x8FEF /* GL_UNSIGNED_INT8_VEC4_NV */:
      return "GL_UNSIGNED_INT8_VEC4_NV (0x8FEF)";
    case 0x8FF0 /* GL_UNSIGNED_INT16_NV */:
      return "GL_UNSIGNED_INT16_NV (0x8FF0)";
    case 0x8FF1 /* GL_UNSIGNED_INT16_VEC2_NV */:
      return "GL_UNSIGNED_INT16_VEC2_NV (0x8FF1)";
    case 0x8FF2 /* GL_UNSIGNED_INT16_VEC3_NV */:
      return "GL_UNSIGNED_INT16_VEC3_NV (0x8FF2)";
    case 0x8FF3 /* GL_UNSIGNED_INT16_VEC4_NV */:
      return "GL_UNSIGNED_INT16_VEC4_NV (0x8FF3)";
    case 0x8FF5 /* GL_UNSIGNED_INT64_VEC2_NV */:
      return "GL_UNSIGNED_INT64_VEC2_NV (0x8FF5)";
    case 0x8FF6 /* GL_UNSIGNED_INT64_VEC3_NV */:
      return "GL_UNSIGNED_INT64_VEC3_NV (0x8FF6)";
    case 0x8FF7 /* GL_UNSIGNED_INT64_VEC4_NV */:
      return "GL_UNSIGNED_INT64_VEC4_NV (0x8FF7)";
    case 0x8FF8 /* GL_FLOAT16_NV */:
      return "GL_FLOAT16_NV (0x8FF8)";
    case 0x8FF9 /* GL_FLOAT16_VEC2_NV */:
      return "GL_FLOAT16_VEC2_NV (0x8FF9)";
    case 0x8FFA /* GL_FLOAT16_VEC3_NV */:
      return "GL_FLOAT16_VEC3_NV (0x8FFA)";
    case 0x8FFB /* GL_FLOAT16_VEC4_NV */:
      return "GL_FLOAT16_VEC4_NV (0x8FFB)";

      /* ---------------------------- GL_NV_half_float
       * --------------------------- */

      // case 0x140B /* GL_HALF_FLOAT_NV */: return "GL_HALF_FLOAT_NV (0x140B)";

      /* ------------------------ GL_NV_light_max_exponent
       * ----------------------- */

    case 0x8504 /* GL_MAX_SHININESS_NV */:
      return "GL_MAX_SHININESS_NV (0x8504)";
    case 0x8505 /* GL_MAX_SPOT_EXPONENT_NV */:
      return "GL_MAX_SPOT_EXPONENT_NV (0x8505)";

    /* ----------------------- GL_NV_multisample_coverage ----------------------
     */

    // case 0x80A9 /* GL_COVERAGE_SAMPLES_NV */: return "GL_COVERAGE_SAMPLES_NV
    // (0x80A9)";
    case 0x8E20 /* GL_COLOR_SAMPLES_NV */:
      return "GL_COLOR_SAMPLES_NV (0x8E20)";

      /* --------------------- GL_NV_multisample_filter_hint
       * --------------------- */

    case 0x8534 /* GL_MULTISAMPLE_FILTER_HINT_NV */:
      return "GL_MULTISAMPLE_FILTER_HINT_NV (0x8534)";

      /* ------------------------- GL_NV_occlusion_query
       * ------------------------- */

      // case 0x8864 /* GL_PIXEL_COUNTER_BITS_NV */: return
      // "GL_PIXEL_COUNTER_BITS_NV (0x8864)"; case 0x8865 /*
      // GL_CURRENT_OCCLUSION_QUERY_ID_NV */: return
      // "GL_CURRENT_OCCLUSION_QUERY_ID_NV (0x8865)"; case 0x8866 /*
      // GL_PIXEL_COUNT_NV */: return "GL_PIXEL_COUNT_NV (0x8866)"; case 0x8867
      // /* GL_PIXEL_COUNT_AVAILABLE_NV */: return "GL_PIXEL_COUNT_AVAILABLE_NV
      // (0x8867)";

      /* ----------------------- GL_NV_packed_depth_stencil
       * ---------------------- */

      // case 0x84F9 /* GL_DEPTH_STENCIL_NV */: return "GL_DEPTH_STENCIL_NV
      // (0x84F9)"; case 0x84FA /* GL_UNSIGNED_INT_24_8_NV */: return
      // "GL_UNSIGNED_INT_24_8_NV (0x84FA)";

      /* --------------------- GL_NV_parameter_buffer_object
       * --------------------- */

    case 0x8DA0 /* GL_MAX_PROGRAM_PARAMETER_BUFFER_BINDINGS_NV */:
      return "GL_MAX_PROGRAM_PARAMETER_BUFFER_BINDINGS_NV (0x8DA0)";
    case 0x8DA1 /* GL_MAX_PROGRAM_PARAMETER_BUFFER_SIZE_NV */:
      return "GL_MAX_PROGRAM_PARAMETER_BUFFER_SIZE_NV (0x8DA1)";
    case 0x8DA2 /* GL_VERTEX_PROGRAM_PARAMETER_BUFFER_NV */:
      return "GL_VERTEX_PROGRAM_PARAMETER_BUFFER_NV (0x8DA2)";
    case 0x8DA3 /* GL_GEOMETRY_PROGRAM_PARAMETER_BUFFER_NV */:
      return "GL_GEOMETRY_PROGRAM_PARAMETER_BUFFER_NV (0x8DA3)";
    case 0x8DA4 /* GL_FRAGMENT_PROGRAM_PARAMETER_BUFFER_NV */:
      return "GL_FRAGMENT_PROGRAM_PARAMETER_BUFFER_NV (0x8DA4)";

    /* --------------------- GL_NV_parameter_buffer_object2 --------------------
     */

    /* -------------------------- GL_NV_path_rendering -------------------------
     */

    // case 0x00 /* GL_CLOSE_PATH_NV */: return "GL_CLOSE_PATH_NV (0x00)";
    // case 0x01 /* GL_BOLD_BIT_NV */: return "GL_BOLD_BIT_NV (0x01)";
    // case 0x01 /* GL_GLYPH_WIDTH_BIT_NV */: return "GL_GLYPH_WIDTH_BIT_NV
    // (0x01)"; case 0x02 /* GL_GLYPH_HEIGHT_BIT_NV */: return
    // "GL_GLYPH_HEIGHT_BIT_NV (0x02)"; case 0x02 /* GL_ITALIC_BIT_NV */: return
    // "GL_ITALIC_BIT_NV (0x02)"; case 0x02 /* GL_MOVE_TO_NV */: return
    // "GL_MOVE_TO_NV (0x02)"; case 0x03 /* GL_RELATIVE_MOVE_TO_NV */: return
    // "GL_RELATIVE_MOVE_TO_NV (0x03)"; case 0x04 /* GL_LINE_TO_NV */: return
    // "GL_LINE_TO_NV (0x04)"; case 0x04 /* GL_GLYPH_HORIZONTAL_BEARING_X_BIT_NV
    // */: return "GL_GLYPH_HORIZONTAL_BEARING_X_BIT_NV (0x04)"; case 0x05 /*
    // GL_RELATIVE_LINE_TO_NV */: return "GL_RELATIVE_LINE_TO_NV (0x05)"; case
    // 0x06 /* GL_HORIZONTAL_LINE_TO_NV */: return "GL_HORIZONTAL_LINE_TO_NV
    // (0x06)"; case 0x07 /* GL_RELATIVE_HORIZONTAL_LINE_TO_NV */: return
    // "GL_RELATIVE_HORIZONTAL_LINE_TO_NV (0x07)"; case 0x08 /*
    // GL_GLYPH_HORIZONTAL_BEARING_Y_BIT_NV */: return
    // "GL_GLYPH_HORIZONTAL_BEARING_Y_BIT_NV (0x08)"; case 0x08 /*
    // GL_VERTICAL_LINE_TO_NV */: return "GL_VERTICAL_LINE_TO_NV (0x08)"; case
    // 0x09 /* GL_RELATIVE_VERTICAL_LINE_TO_NV */: return
    // "GL_RELATIVE_VERTICAL_LINE_TO_NV (0x09)"; case 0x0A /*
    // GL_QUADRATIC_CURVE_TO_NV */: return "GL_QUADRATIC_CURVE_TO_NV (0x0A)";
    // case 0x0B /* GL_RELATIVE_QUADRATIC_CURVE_TO_NV */: return
    // "GL_RELATIVE_QUADRATIC_CURVE_TO_NV (0x0B)"; case 0x0C /*
    // GL_CUBIC_CURVE_TO_NV */: return "GL_CUBIC_CURVE_TO_NV (0x0C)"; case 0x0D
    // /* GL_RELATIVE_CUBIC_CURVE_TO_NV */: return "GL_RELATIVE_CUBIC_CURVE_TO_NV
    // (0x0D)"; case 0x0E /* GL_SMOOTH_QUADRATIC_CURVE_TO_NV */: return
    // "GL_SMOOTH_QUADRATIC_CURVE_TO_NV (0x0E)";
    case 0x0F /* GL_RELATIVE_SMOOTH_QUADRATIC_CURVE_TO_NV */:
      return "GL_RELATIVE_SMOOTH_QUADRATIC_CURVE_TO_NV (0x0F)";
    // case 0x10 /* GL_GLYPH_HORIZONTAL_BEARING_ADVANCE_BIT_NV */: return
    // "GL_GLYPH_HORIZONTAL_BEARING_ADVANCE_BIT_NV (0x10)"; case 0x10 /*
    // GL_SMOOTH_CUBIC_CURVE_TO_NV */: return "GL_SMOOTH_CUBIC_CURVE_TO_NV
    // (0x10)";
    case 0x11 /* GL_RELATIVE_SMOOTH_CUBIC_CURVE_TO_NV */:
      return "GL_RELATIVE_SMOOTH_CUBIC_CURVE_TO_NV (0x11)";
    case 0x12 /* GL_SMALL_CCW_ARC_TO_NV */:
      return "GL_SMALL_CCW_ARC_TO_NV (0x12)";
    case 0x13 /* GL_RELATIVE_SMALL_CCW_ARC_TO_NV */:
      return "GL_RELATIVE_SMALL_CCW_ARC_TO_NV (0x13)";
    case 0x14 /* GL_SMALL_CW_ARC_TO_NV */:
      return "GL_SMALL_CW_ARC_TO_NV (0x14)";
    case 0x15 /* GL_RELATIVE_SMALL_CW_ARC_TO_NV */:
      return "GL_RELATIVE_SMALL_CW_ARC_TO_NV (0x15)";
    case 0x16 /* GL_LARGE_CCW_ARC_TO_NV */:
      return "GL_LARGE_CCW_ARC_TO_NV (0x16)";
    case 0x17 /* GL_RELATIVE_LARGE_CCW_ARC_TO_NV */:
      return "GL_RELATIVE_LARGE_CCW_ARC_TO_NV (0x17)";
    case 0x18 /* GL_LARGE_CW_ARC_TO_NV */:
      return "GL_LARGE_CW_ARC_TO_NV (0x18)";
    case 0x19 /* GL_RELATIVE_LARGE_CW_ARC_TO_NV */:
      return "GL_RELATIVE_LARGE_CW_ARC_TO_NV (0x19)";
    // case 0x20 /* GL_GLYPH_VERTICAL_BEARING_X_BIT_NV */: return
    // "GL_GLYPH_VERTICAL_BEARING_X_BIT_NV (0x20)"; case 0x40 /*
    // GL_GLYPH_VERTICAL_BEARING_Y_BIT_NV */: return
    // "GL_GLYPH_VERTICAL_BEARING_Y_BIT_NV (0x40)"; case 0x80 /*
    // GL_GLYPH_VERTICAL_BEARING_ADVANCE_BIT_NV */: return
    // "GL_GLYPH_VERTICAL_BEARING_ADVANCE_BIT_NV (0x80)";
    case 0xF0 /* GL_RESTART_PATH_NV */:
      return "GL_RESTART_PATH_NV (0xF0)";
    case 0xF2 /* GL_DUP_FIRST_CUBIC_CURVE_TO_NV */:
      return "GL_DUP_FIRST_CUBIC_CURVE_TO_NV (0xF2)";
    case 0xF4 /* GL_DUP_LAST_CUBIC_CURVE_TO_NV */:
      return "GL_DUP_LAST_CUBIC_CURVE_TO_NV (0xF4)";
    case 0xF6 /* GL_RECT_NV */:
      return "GL_RECT_NV (0xF6)";
    case 0xF8 /* GL_CIRCULAR_CCW_ARC_TO_NV */:
      return "GL_CIRCULAR_CCW_ARC_TO_NV (0xF8)";
    case 0xFA /* GL_CIRCULAR_CW_ARC_TO_NV */:
      return "GL_CIRCULAR_CW_ARC_TO_NV (0xFA)";
    case 0xFC /* GL_CIRCULAR_TANGENT_ARC_TO_NV */:
      return "GL_CIRCULAR_TANGENT_ARC_TO_NV (0xFC)";
    case 0xFE /* GL_ARC_TO_NV */:
      return "GL_ARC_TO_NV (0xFE)";
    case 0xFF /* GL_RELATIVE_ARC_TO_NV */:
      return "GL_RELATIVE_ARC_TO_NV (0xFF)";
    // case 0x100 /* GL_GLYPH_HAS_KERNING_BIT_NV */: return
    // "GL_GLYPH_HAS_KERNING_BIT_NV (0x100)";
    case 0x852C /* GL_PRIMARY_COLOR_NV */:
      return "GL_PRIMARY_COLOR_NV (0x852C)";
    case 0x852D /* GL_SECONDARY_COLOR_NV */:
      return "GL_SECONDARY_COLOR_NV (0x852D)";
    // case 0x8577 /* GL_PRIMARY_COLOR */: return "GL_PRIMARY_COLOR (0x8577)";
    case 0x9070 /* GL_PATH_FORMAT_SVG_NV */:
      return "GL_PATH_FORMAT_SVG_NV (0x9070)";
    case 0x9071 /* GL_PATH_FORMAT_PS_NV */:
      return "GL_PATH_FORMAT_PS_NV (0x9071)";
    case 0x9072 /* GL_STANDARD_FONT_NAME_NV */:
      return "GL_STANDARD_FONT_NAME_NV (0x9072)";
    case 0x9073 /* GL_SYSTEM_FONT_NAME_NV */:
      return "GL_SYSTEM_FONT_NAME_NV (0x9073)";
    case 0x9074 /* GL_FILE_NAME_NV */:
      return "GL_FILE_NAME_NV (0x9074)";
    case 0x9075 /* GL_PATH_STROKE_WIDTH_NV */:
      return "GL_PATH_STROKE_WIDTH_NV (0x9075)";
    case 0x9076 /* GL_PATH_END_CAPS_NV */:
      return "GL_PATH_END_CAPS_NV (0x9076)";
    case 0x9077 /* GL_PATH_INITIAL_END_CAP_NV */:
      return "GL_PATH_INITIAL_END_CAP_NV (0x9077)";
    case 0x9078 /* GL_PATH_TERMINAL_END_CAP_NV */:
      return "GL_PATH_TERMINAL_END_CAP_NV (0x9078)";
    case 0x9079 /* GL_PATH_JOIN_STYLE_NV */:
      return "GL_PATH_JOIN_STYLE_NV (0x9079)";
    case 0x907A /* GL_PATH_MITER_LIMIT_NV */:
      return "GL_PATH_MITER_LIMIT_NV (0x907A)";
    case 0x907B /* GL_PATH_DASH_CAPS_NV */:
      return "GL_PATH_DASH_CAPS_NV (0x907B)";
    case 0x907C /* GL_PATH_INITIAL_DASH_CAP_NV */:
      return "GL_PATH_INITIAL_DASH_CAP_NV (0x907C)";
    case 0x907D /* GL_PATH_TERMINAL_DASH_CAP_NV */:
      return "GL_PATH_TERMINAL_DASH_CAP_NV (0x907D)";
    case 0x907E /* GL_PATH_DASH_OFFSET_NV */:
      return "GL_PATH_DASH_OFFSET_NV (0x907E)";
    case 0x907F /* GL_PATH_CLIENT_LENGTH_NV */:
      return "GL_PATH_CLIENT_LENGTH_NV (0x907F)";
    case 0x9080 /* GL_PATH_FILL_MODE_NV */:
      return "GL_PATH_FILL_MODE_NV (0x9080)";
    case 0x9081 /* GL_PATH_FILL_MASK_NV */:
      return "GL_PATH_FILL_MASK_NV (0x9081)";
    case 0x9082 /* GL_PATH_FILL_COVER_MODE_NV */:
      return "GL_PATH_FILL_COVER_MODE_NV (0x9082)";
    case 0x9083 /* GL_PATH_STROKE_COVER_MODE_NV */:
      return "GL_PATH_STROKE_COVER_MODE_NV (0x9083)";
    case 0x9084 /* GL_PATH_STROKE_MASK_NV */:
      return "GL_PATH_STROKE_MASK_NV (0x9084)";
    case 0x9088 /* GL_COUNT_UP_NV */:
      return "GL_COUNT_UP_NV (0x9088)";
    case 0x9089 /* GL_COUNT_DOWN_NV */:
      return "GL_COUNT_DOWN_NV (0x9089)";
    case 0x908A /* GL_PATH_OBJECT_BOUNDING_BOX_NV */:
      return "GL_PATH_OBJECT_BOUNDING_BOX_NV (0x908A)";
    case 0x908B /* GL_CONVEX_HULL_NV */:
      return "GL_CONVEX_HULL_NV (0x908B)";
    case 0x908D /* GL_BOUNDING_BOX_NV */:
      return "GL_BOUNDING_BOX_NV (0x908D)";
    case 0x908E /* GL_TRANSLATE_X_NV */:
      return "GL_TRANSLATE_X_NV (0x908E)";
    case 0x908F /* GL_TRANSLATE_Y_NV */:
      return "GL_TRANSLATE_Y_NV (0x908F)";
    case 0x9090 /* GL_TRANSLATE_2D_NV */:
      return "GL_TRANSLATE_2D_NV (0x9090)";
    case 0x9091 /* GL_TRANSLATE_3D_NV */:
      return "GL_TRANSLATE_3D_NV (0x9091)";
    case 0x9092 /* GL_AFFINE_2D_NV */:
      return "GL_AFFINE_2D_NV (0x9092)";
    case 0x9094 /* GL_AFFINE_3D_NV */:
      return "GL_AFFINE_3D_NV (0x9094)";
    case 0x9096 /* GL_TRANSPOSE_AFFINE_2D_NV */:
      return "GL_TRANSPOSE_AFFINE_2D_NV (0x9096)";
    case 0x9098 /* GL_TRANSPOSE_AFFINE_3D_NV */:
      return "GL_TRANSPOSE_AFFINE_3D_NV (0x9098)";
    case 0x909A /* GL_UTF8_NV */:
      return "GL_UTF8_NV (0x909A)";
    case 0x909B /* GL_UTF16_NV */:
      return "GL_UTF16_NV (0x909B)";
    case 0x909C /* GL_BOUNDING_BOX_OF_BOUNDING_BOXES_NV */:
      return "GL_BOUNDING_BOX_OF_BOUNDING_BOXES_NV (0x909C)";
    case 0x909D /* GL_PATH_COMMAND_COUNT_NV */:
      return "GL_PATH_COMMAND_COUNT_NV (0x909D)";
    case 0x909E /* GL_PATH_COORD_COUNT_NV */:
      return "GL_PATH_COORD_COUNT_NV (0x909E)";
    case 0x909F /* GL_PATH_DASH_ARRAY_COUNT_NV */:
      return "GL_PATH_DASH_ARRAY_COUNT_NV (0x909F)";
    case 0x90A0 /* GL_PATH_COMPUTED_LENGTH_NV */:
      return "GL_PATH_COMPUTED_LENGTH_NV (0x90A0)";
    case 0x90A1 /* GL_PATH_FILL_BOUNDING_BOX_NV */:
      return "GL_PATH_FILL_BOUNDING_BOX_NV (0x90A1)";
    case 0x90A2 /* GL_PATH_STROKE_BOUNDING_BOX_NV */:
      return "GL_PATH_STROKE_BOUNDING_BOX_NV (0x90A2)";
    case 0x90A3 /* GL_SQUARE_NV */:
      return "GL_SQUARE_NV (0x90A3)";
    case 0x90A4 /* GL_ROUND_NV */:
      return "GL_ROUND_NV (0x90A4)";
    case 0x90A5 /* GL_TRIANGULAR_NV */:
      return "GL_TRIANGULAR_NV (0x90A5)";
    case 0x90A6 /* GL_BEVEL_NV */:
      return "GL_BEVEL_NV (0x90A6)";
    case 0x90A7 /* GL_MITER_REVERT_NV */:
      return "GL_MITER_REVERT_NV (0x90A7)";
    case 0x90A8 /* GL_MITER_TRUNCATE_NV */:
      return "GL_MITER_TRUNCATE_NV (0x90A8)";
    case 0x90A9 /* GL_SKIP_MISSING_GLYPH_NV */:
      return "GL_SKIP_MISSING_GLYPH_NV (0x90A9)";
    case 0x90AA /* GL_USE_MISSING_GLYPH_NV */:
      return "GL_USE_MISSING_GLYPH_NV (0x90AA)";
    case 0x90AB /* GL_PATH_ERROR_POSITION_NV */:
      return "GL_PATH_ERROR_POSITION_NV (0x90AB)";
    case 0x90AC /* GL_PATH_FOG_GEN_MODE_NV */:
      return "GL_PATH_FOG_GEN_MODE_NV (0x90AC)";
    case 0x90AD /* GL_ACCUM_ADJACENT_PAIRS_NV */:
      return "GL_ACCUM_ADJACENT_PAIRS_NV (0x90AD)";
    case 0x90AE /* GL_ADJACENT_PAIRS_NV */:
      return "GL_ADJACENT_PAIRS_NV (0x90AE)";
    case 0x90AF /* GL_FIRST_TO_REST_NV */:
      return "GL_FIRST_TO_REST_NV (0x90AF)";
    case 0x90B0 /* GL_PATH_GEN_MODE_NV */:
      return "GL_PATH_GEN_MODE_NV (0x90B0)";
    case 0x90B1 /* GL_PATH_GEN_COEFF_NV */:
      return "GL_PATH_GEN_COEFF_NV (0x90B1)";
    case 0x90B2 /* GL_PATH_GEN_COLOR_FORMAT_NV */:
      return "GL_PATH_GEN_COLOR_FORMAT_NV (0x90B2)";
    case 0x90B3 /* GL_PATH_GEN_COMPONENTS_NV */:
      return "GL_PATH_GEN_COMPONENTS_NV (0x90B3)";
    case 0x90B4 /* GL_PATH_DASH_OFFSET_RESET_NV */:
      return "GL_PATH_DASH_OFFSET_RESET_NV (0x90B4)";
    case 0x90B5 /* GL_MOVE_TO_RESETS_NV */:
      return "GL_MOVE_TO_RESETS_NV (0x90B5)";
    case 0x90B6 /* GL_MOVE_TO_CONTINUES_NV */:
      return "GL_MOVE_TO_CONTINUES_NV (0x90B6)";
    case 0x90B7 /* GL_PATH_STENCIL_FUNC_NV */:
      return "GL_PATH_STENCIL_FUNC_NV (0x90B7)";
    case 0x90B8 /* GL_PATH_STENCIL_REF_NV */:
      return "GL_PATH_STENCIL_REF_NV (0x90B8)";
    case 0x90B9 /* GL_PATH_STENCIL_VALUE_MASK_NV */:
      return "GL_PATH_STENCIL_VALUE_MASK_NV (0x90B9)";
    case 0x90BD /* GL_PATH_STENCIL_DEPTH_OFFSET_FACTOR_NV */:
      return "GL_PATH_STENCIL_DEPTH_OFFSET_FACTOR_NV (0x90BD)";
    case 0x90BE /* GL_PATH_STENCIL_DEPTH_OFFSET_UNITS_NV */:
      return "GL_PATH_STENCIL_DEPTH_OFFSET_UNITS_NV (0x90BE)";
    case 0x90BF /* GL_PATH_COVER_DEPTH_FUNC_NV */:
      return "GL_PATH_COVER_DEPTH_FUNC_NV (0x90BF)";
    // case 0x00010000 /* GL_FONT_X_MIN_BOUNDS_BIT_NV */: return
    // "GL_FONT_X_MIN_BOUNDS_BIT_NV (0x00010000)"; case 0x00020000 /*
    // GL_FONT_Y_MIN_BOUNDS_BIT_NV */: return "GL_FONT_Y_MIN_BOUNDS_BIT_NV
    // (0x00020000)"; case 0x00040000 /* GL_FONT_X_MAX_BOUNDS_BIT_NV */: return
    // "GL_FONT_X_MAX_BOUNDS_BIT_NV (0x00040000)"; case 0x00080000 /*
    // GL_FONT_Y_MAX_BOUNDS_BIT_NV */: return "GL_FONT_Y_MAX_BOUNDS_BIT_NV
    // (0x00080000)";
    case 0x00100000 /* GL_FONT_UNITS_PER_EM_BIT_NV */:
      return "GL_FONT_UNITS_PER_EM_BIT_NV (0x00100000)";
    case 0x00200000 /* GL_FONT_ASCENDER_BIT_NV */:
      return "GL_FONT_ASCENDER_BIT_NV (0x00200000)";
    case 0x00400000 /* GL_FONT_DESCENDER_BIT_NV */:
      return "GL_FONT_DESCENDER_BIT_NV (0x00400000)";
    case 0x00800000 /* GL_FONT_HEIGHT_BIT_NV */:
      return "GL_FONT_HEIGHT_BIT_NV (0x00800000)";
    case 0x01000000 /* GL_FONT_MAX_ADVANCE_WIDTH_BIT_NV */:
      return "GL_FONT_MAX_ADVANCE_WIDTH_BIT_NV (0x01000000)";
    case 0x02000000 /* GL_FONT_MAX_ADVANCE_HEIGHT_BIT_NV */:
      return "GL_FONT_MAX_ADVANCE_HEIGHT_BIT_NV (0x02000000)";
    case 0x04000000 /* GL_FONT_UNDERLINE_POSITION_BIT_NV */:
      return "GL_FONT_UNDERLINE_POSITION_BIT_NV (0x04000000)";
    case 0x08000000 /* GL_FONT_UNDERLINE_THICKNESS_BIT_NV */:
      return "GL_FONT_UNDERLINE_THICKNESS_BIT_NV (0x08000000)";
    case 0x10000000 /* GL_FONT_HAS_KERNING_BIT_NV */:
      return "GL_FONT_HAS_KERNING_BIT_NV (0x10000000)";

      /* ------------------------- GL_NV_pixel_data_range
       * ------------------------ */

    case 0x8878 /* GL_WRITE_PIXEL_DATA_RANGE_NV */:
      return "GL_WRITE_PIXEL_DATA_RANGE_NV (0x8878)";
    case 0x8879 /* GL_READ_PIXEL_DATA_RANGE_NV */:
      return "GL_READ_PIXEL_DATA_RANGE_NV (0x8879)";
    case 0x887A /* GL_WRITE_PIXEL_DATA_RANGE_LENGTH_NV */:
      return "GL_WRITE_PIXEL_DATA_RANGE_LENGTH_NV (0x887A)";
    case 0x887B /* GL_READ_PIXEL_DATA_RANGE_LENGTH_NV */:
      return "GL_READ_PIXEL_DATA_RANGE_LENGTH_NV (0x887B)";
    case 0x887C /* GL_WRITE_PIXEL_DATA_RANGE_POINTER_NV */:
      return "GL_WRITE_PIXEL_DATA_RANGE_POINTER_NV (0x887C)";
    case 0x887D /* GL_READ_PIXEL_DATA_RANGE_POINTER_NV */:
      return "GL_READ_PIXEL_DATA_RANGE_POINTER_NV (0x887D)";

    /* --------------------------- GL_NV_point_sprite --------------------------
     */

    // case 0x8861 /* GL_POINT_SPRITE_NV */: return "GL_POINT_SPRITE_NV
    // (0x8861)"; case 0x8862 /* GL_COORD_REPLACE_NV */: return
    // "GL_COORD_REPLACE_NV (0x8862)";
    case 0x8863 /* GL_POINT_SPRITE_R_MODE_NV */:
      return "GL_POINT_SPRITE_R_MODE_NV (0x8863)";

      /* -------------------------- GL_NV_present_video
       * -------------------------- */

    case 0x8E26 /* GL_FRAME_NV */:
      return "GL_FRAME_NV (0x8E26)";
    case 0x8E27 /* GL_FIELDS_NV */:
      return "GL_FIELDS_NV (0x8E27)";
    // case 0x8E28 /* GL_CURRENT_TIME_NV */: return "GL_CURRENT_TIME_NV
    // (0x8E28)";
    case 0x8E29 /* GL_NUM_FILL_STREAMS_NV */:
      return "GL_NUM_FILL_STREAMS_NV (0x8E29)";
    case 0x8E2A /* GL_PRESENT_TIME_NV */:
      return "GL_PRESENT_TIME_NV (0x8E2A)";
    case 0x8E2B /* GL_PRESENT_DURATION_NV */:
      return "GL_PRESENT_DURATION_NV (0x8E2B)"; /* ------------------------
                                                   GL_NV_primitive_restart
                                                   ------------------------ */

    case 0x8558 /* GL_PRIMITIVE_RESTART_NV */:
      return "GL_PRIMITIVE_RESTART_NV (0x8558)";
    case 0x8559 /* GL_PRIMITIVE_RESTART_INDEX_NV */:
      return "GL_PRIMITIVE_RESTART_INDEX_NV (0x8559)";

      /* ------------------------ GL_NV_register_combiners
       * ----------------------- */

    case 0x8522 /* GL_REGISTER_COMBINERS_NV */:
      return "GL_REGISTER_COMBINERS_NV (0x8522)";
    case 0x8523 /* GL_VARIABLE_A_NV */:
      return "GL_VARIABLE_A_NV (0x8523)";
    case 0x8524 /* GL_VARIABLE_B_NV */:
      return "GL_VARIABLE_B_NV (0x8524)";
    case 0x8525 /* GL_VARIABLE_C_NV */:
      return "GL_VARIABLE_C_NV (0x8525)";
    case 0x8526 /* GL_VARIABLE_D_NV */:
      return "GL_VARIABLE_D_NV (0x8526)";
    case 0x8527 /* GL_VARIABLE_E_NV */:
      return "GL_VARIABLE_E_NV (0x8527)";
    case 0x8528 /* GL_VARIABLE_F_NV */:
      return "GL_VARIABLE_F_NV (0x8528)";
    case 0x8529 /* GL_VARIABLE_G_NV */:
      return "GL_VARIABLE_G_NV (0x8529)";
    case 0x852A /* GL_CONSTANT_COLOR0_NV */:
      return "GL_CONSTANT_COLOR0_NV (0x852A)";
    case 0x852B /* GL_CONSTANT_COLOR1_NV */:
      return "GL_CONSTANT_COLOR1_NV (0x852B)";
    // case 0x852C /* GL_PRIMARY_COLOR_NV */: return "GL_PRIMARY_COLOR_NV
    // (0x852C)"; case 0x852D /* GL_SECONDARY_COLOR_NV */: return
    // "GL_SECONDARY_COLOR_NV (0x852D)";
    case 0x852E /* GL_SPARE0_NV */:
      return "GL_SPARE0_NV (0x852E)";
    case 0x852F /* GL_SPARE1_NV */:
      return "GL_SPARE1_NV (0x852F)";
    case 0x8530 /* GL_DISCARD_NV */:
      return "GL_DISCARD_NV (0x8530)";
    case 0x8531 /* GL_E_TIMES_F_NV */:
      return "GL_E_TIMES_F_NV (0x8531)";
    case 0x8532 /* GL_SPARE0_PLUS_SECONDARY_COLOR_NV */:
      return "GL_SPARE0_PLUS_SECONDARY_COLOR_NV (0x8532)";
    case 0x8536 /* GL_UNSIGNED_IDENTITY_NV */:
      return "GL_UNSIGNED_IDENTITY_NV (0x8536)";
    case 0x8537 /* GL_UNSIGNED_INVERT_NV */:
      return "GL_UNSIGNED_INVERT_NV (0x8537)";
    case 0x8538 /* GL_EXPAND_NORMAL_NV */:
      return "GL_EXPAND_NORMAL_NV (0x8538)";
    case 0x8539 /* GL_EXPAND_NEGATE_NV */:
      return "GL_EXPAND_NEGATE_NV (0x8539)";
    case 0x853A /* GL_HALF_BIAS_NORMAL_NV */:
      return "GL_HALF_BIAS_NORMAL_NV (0x853A)";
    case 0x853B /* GL_HALF_BIAS_NEGATE_NV */:
      return "GL_HALF_BIAS_NEGATE_NV (0x853B)";
    case 0x853C /* GL_SIGNED_IDENTITY_NV */:
      return "GL_SIGNED_IDENTITY_NV (0x853C)";
    case 0x853D /* GL_SIGNED_NEGATE_NV */:
      return "GL_SIGNED_NEGATE_NV (0x853D)";
    case 0x853E /* GL_SCALE_BY_TWO_NV */:
      return "GL_SCALE_BY_TWO_NV (0x853E)";
    case 0x853F /* GL_SCALE_BY_FOUR_NV */:
      return "GL_SCALE_BY_FOUR_NV (0x853F)";
    case 0x8540 /* GL_SCALE_BY_ONE_HALF_NV */:
      return "GL_SCALE_BY_ONE_HALF_NV (0x8540)";
    case 0x8541 /* GL_BIAS_BY_NEGATIVE_ONE_HALF_NV */:
      return "GL_BIAS_BY_NEGATIVE_ONE_HALF_NV (0x8541)";
    case 0x8542 /* GL_COMBINER_INPUT_NV */:
      return "GL_COMBINER_INPUT_NV (0x8542)";
    case 0x8543 /* GL_COMBINER_MAPPING_NV */:
      return "GL_COMBINER_MAPPING_NV (0x8543)";
    case 0x8544 /* GL_COMBINER_COMPONENT_USAGE_NV */:
      return "GL_COMBINER_COMPONENT_USAGE_NV (0x8544)";
    case 0x8545 /* GL_COMBINER_AB_DOT_PRODUCT_NV */:
      return "GL_COMBINER_AB_DOT_PRODUCT_NV (0x8545)";
    case 0x8546 /* GL_COMBINER_CD_DOT_PRODUCT_NV */:
      return "GL_COMBINER_CD_DOT_PRODUCT_NV (0x8546)";
    case 0x8547 /* GL_COMBINER_MUX_SUM_NV */:
      return "GL_COMBINER_MUX_SUM_NV (0x8547)";
    case 0x8548 /* GL_COMBINER_SCALE_NV */:
      return "GL_COMBINER_SCALE_NV (0x8548)";
    case 0x8549 /* GL_COMBINER_BIAS_NV */:
      return "GL_COMBINER_BIAS_NV (0x8549)";
    case 0x854A /* GL_COMBINER_AB_OUTPUT_NV */:
      return "GL_COMBINER_AB_OUTPUT_NV (0x854A)";
    case 0x854B /* GL_COMBINER_CD_OUTPUT_NV */:
      return "GL_COMBINER_CD_OUTPUT_NV (0x854B)";
    case 0x854C /* GL_COMBINER_SUM_OUTPUT_NV */:
      return "GL_COMBINER_SUM_OUTPUT_NV (0x854C)";
    case 0x854D /* GL_MAX_GENERAL_COMBINERS_NV */:
      return "GL_MAX_GENERAL_COMBINERS_NV (0x854D)";
    case 0x854E /* GL_NUM_GENERAL_COMBINERS_NV */:
      return "GL_NUM_GENERAL_COMBINERS_NV (0x854E)";
    case 0x854F /* GL_COLOR_SUM_CLAMP_NV */:
      return "GL_COLOR_SUM_CLAMP_NV (0x854F)";
    case 0x8550 /* GL_COMBINER0_NV */:
      return "GL_COMBINER0_NV (0x8550)";
    case 0x8551 /* GL_COMBINER1_NV */:
      return "GL_COMBINER1_NV (0x8551)";
    case 0x8552 /* GL_COMBINER2_NV */:
      return "GL_COMBINER2_NV (0x8552)";
    case 0x8553 /* GL_COMBINER3_NV */:
      return "GL_COMBINER3_NV (0x8553)";
    case 0x8554 /* GL_COMBINER4_NV */:
      return "GL_COMBINER4_NV (0x8554)";
    case 0x8555 /* GL_COMBINER5_NV */:
      return "GL_COMBINER5_NV (0x8555)";
    case 0x8556 /* GL_COMBINER6_NV */:
      return "GL_COMBINER6_NV (0x8556)";
    case 0x8557 /* GL_COMBINER7_NV */:
      return "GL_COMBINER7_NV (0x8557)";

      /* ----------------------- GL_NV_register_combiners2
       * ----------------------- */

    case 0x8535 /* GL_PER_STAGE_CONSTANTS_NV */:
      return "GL_PER_STAGE_CONSTANTS_NV (0x8535)";

      /* ----------------------- GL_NV_shader_atomic_float
       * ----------------------- */

      /* ------------------------ GL_NV_shader_buffer_load
       * ----------------------- */

    case 0x8F1D /* GL_BUFFER_GPU_ADDRESS_NV */:
      return "GL_BUFFER_GPU_ADDRESS_NV (0x8F1D)";
    case 0x8F34 /* GL_GPU_ADDRESS_NV */:
      return "GL_GPU_ADDRESS_NV (0x8F34)";
    case 0x8F35 /* GL_MAX_SHADER_BUFFER_ADDRESS_NV */:
      return "GL_MAX_SHADER_BUFFER_ADDRESS_NV (0x8F35)";

      /* ---------------------- GL_NV_tessellation_program5
       * ---------------------- */

    case 0x86D8 /* GL_MAX_PROGRAM_PATCH_ATTRIBS_NV */:
      return "GL_MAX_PROGRAM_PATCH_ATTRIBS_NV (0x86D8)";
    case 0x891E /* GL_TESS_CONTROL_PROGRAM_NV */:
      return "GL_TESS_CONTROL_PROGRAM_NV (0x891E)";
    case 0x891F /* GL_TESS_EVALUATION_PROGRAM_NV */:
      return "GL_TESS_EVALUATION_PROGRAM_NV (0x891F)";
    case 0x8C74 /* GL_TESS_CONTROL_PROGRAM_PARAMETER_BUFFER_NV */:
      return "GL_TESS_CONTROL_PROGRAM_PARAMETER_BUFFER_NV (0x8C74)";
    case 0x8C75 /* GL_TESS_EVALUATION_PROGRAM_PARAMETER_BUFFER_NV */:
      return "GL_TESS_EVALUATION_PROGRAM_PARAMETER_BUFFER_NV (0x8C75)";

      /* -------------------------- GL_NV_texgen_emboss
       * -------------------------- */

    case 0x855D /* GL_EMBOSS_LIGHT_NV */:
      return "GL_EMBOSS_LIGHT_NV (0x855D)";
    case 0x855E /* GL_EMBOSS_CONSTANT_NV */:
      return "GL_EMBOSS_CONSTANT_NV (0x855E)";
    case 0x855F /* GL_EMBOSS_MAP_NV */:
      return "GL_EMBOSS_MAP_NV (0x855F)";

      /* ------------------------ GL_NV_texgen_reflection
       * ------------------------ */

      // case 0x8511 /* GL_NORMAL_MAP_NV */: return "GL_NORMAL_MAP_NV (0x8511)";
      // case 0x8512 /* GL_REFLECTION_MAP_NV */: return "GL_REFLECTION_MAP_NV
      // (0x8512)";

      /* ------------------------- GL_NV_texture_barrier
       * ------------------------- */

      /* --------------------- GL_NV_texture_compression_vtc
       * --------------------- */

      /* ----------------------- GL_NV_texture_env_combine4
       * ---------------------- */

    case 0x8503 /* GL_COMBINE4_NV */:
      return "GL_COMBINE4_NV (0x8503)";
    case 0x8583 /* GL_SOURCE3_RGB_NV */:
      return "GL_SOURCE3_RGB_NV (0x8583)";
    case 0x858B /* GL_SOURCE3_ALPHA_NV */:
      return "GL_SOURCE3_ALPHA_NV (0x858B)";
    case 0x8593 /* GL_OPERAND3_RGB_NV */:
      return "GL_OPERAND3_RGB_NV (0x8593)";
    case 0x859B /* GL_OPERAND3_ALPHA_NV */:
      return "GL_OPERAND3_ALPHA_NV (0x859B)";

      /* ---------------------- GL_NV_texture_expand_normal
       * ---------------------- */

    case 0x888F /* GL_TEXTURE_UNSIGNED_REMAP_MODE_NV */:
      return "GL_TEXTURE_UNSIGNED_REMAP_MODE_NV (0x888F)";

      /* ----------------------- GL_NV_texture_multisample
       * ----------------------- */

    case 0x9045 /* GL_TEXTURE_COVERAGE_SAMPLES_NV */:
      return "GL_TEXTURE_COVERAGE_SAMPLES_NV (0x9045)";
    case 0x9046 /* GL_TEXTURE_COLOR_SAMPLES_NV */:
      return "GL_TEXTURE_COLOR_SAMPLES_NV (0x9046)"; /* ------------------------
                                                        GL_NV_texture_rectangle
                                                        ------------------------
                                                      */

      // case 0x84F5 /* GL_TEXTURE_RECTANGLE_NV */: return
      // "GL_TEXTURE_RECTANGLE_NV (0x84F5)"; case 0x84F6 /*
      // GL_TEXTURE_BINDING_RECTANGLE_NV */: return
      // "GL_TEXTURE_BINDING_RECTANGLE_NV (0x84F6)"; case 0x84F7 /*
      // GL_PROXY_TEXTURE_RECTANGLE_NV */: return "GL_PROXY_TEXTURE_RECTANGLE_NV
      // (0x84F7)"; case 0x84F8 /* GL_MAX_RECTANGLE_TEXTURE_SIZE_NV */: return
      // "GL_MAX_RECTANGLE_TEXTURE_SIZE_NV (0x84F8)";

      /* -------------------------- GL_NV_texture_shader
       * ------------------------- */

    case 0x864C /* GL_OFFSET_TEXTURE_RECTANGLE_NV */:
      return "GL_OFFSET_TEXTURE_RECTANGLE_NV (0x864C)";
    case 0x864D /* GL_OFFSET_TEXTURE_RECTANGLE_SCALE_NV */:
      return "GL_OFFSET_TEXTURE_RECTANGLE_SCALE_NV (0x864D)";
    case 0x864E /* GL_DOT_PRODUCT_TEXTURE_RECTANGLE_NV */:
      return "GL_DOT_PRODUCT_TEXTURE_RECTANGLE_NV (0x864E)";
    case 0x86D9 /* GL_RGBA_UNSIGNED_DOT_PRODUCT_MAPPING_NV */:
      return "GL_RGBA_UNSIGNED_DOT_PRODUCT_MAPPING_NV (0x86D9)";
    case 0x86DA /* GL_UNSIGNED_INT_S8_S8_8_8_NV */:
      return "GL_UNSIGNED_INT_S8_S8_8_8_NV (0x86DA)";
    case 0x86DB /* GL_UNSIGNED_INT_8_8_S8_S8_REV_NV */:
      return "GL_UNSIGNED_INT_8_8_S8_S8_REV_NV (0x86DB)";
    case 0x86DC /* GL_DSDT_MAG_INTENSITY_NV */:
      return "GL_DSDT_MAG_INTENSITY_NV (0x86DC)";
    case 0x86DD /* GL_SHADER_CONSISTENT_NV */:
      return "GL_SHADER_CONSISTENT_NV (0x86DD)";
    case 0x86DE /* GL_TEXTURE_SHADER_NV */:
      return "GL_TEXTURE_SHADER_NV (0x86DE)";
    case 0x86DF /* GL_SHADER_OPERATION_NV */:
      return "GL_SHADER_OPERATION_NV (0x86DF)";
    case 0x86E0 /* GL_CULL_MODES_NV */:
      return "GL_CULL_MODES_NV (0x86E0)";
    case 0x86E1 /* GL_OFFSET_TEXTURE_2D_MATRIX_NV */:
      return "GL_OFFSET_TEXTURE_2D_MATRIX_NV (0x86E1)";
    // case 0x86E1 /* GL_OFFSET_TEXTURE_MATRIX_NV */: return
    // "GL_OFFSET_TEXTURE_MATRIX_NV (0x86E1)";
    case 0x86E2 /* GL_OFFSET_TEXTURE_2D_SCALE_NV */:
      return "GL_OFFSET_TEXTURE_2D_SCALE_NV (0x86E2)";
    // case 0x86E2 /* GL_OFFSET_TEXTURE_SCALE_NV */: return
    // "GL_OFFSET_TEXTURE_SCALE_NV (0x86E2)";
    case 0x86E3 /* GL_OFFSET_TEXTURE_BIAS_NV */:
      return "GL_OFFSET_TEXTURE_BIAS_NV (0x86E3)";
    // case 0x86E3 /* GL_OFFSET_TEXTURE_2D_BIAS_NV */: return
    // "GL_OFFSET_TEXTURE_2D_BIAS_NV (0x86E3)";
    case 0x86E4 /* GL_PREVIOUS_TEXTURE_INPUT_NV */:
      return "GL_PREVIOUS_TEXTURE_INPUT_NV (0x86E4)";
    case 0x86E5 /* GL_CONST_EYE_NV */:
      return "GL_CONST_EYE_NV (0x86E5)";
    case 0x86E6 /* GL_PASS_THROUGH_NV */:
      return "GL_PASS_THROUGH_NV (0x86E6)";
    case 0x86E7 /* GL_CULL_FRAGMENT_NV */:
      return "GL_CULL_FRAGMENT_NV (0x86E7)";
    case 0x86E8 /* GL_OFFSET_TEXTURE_2D_NV */:
      return "GL_OFFSET_TEXTURE_2D_NV (0x86E8)";
    case 0x86E9 /* GL_DEPENDENT_AR_TEXTURE_2D_NV */:
      return "GL_DEPENDENT_AR_TEXTURE_2D_NV (0x86E9)";
    case 0x86EA /* GL_DEPENDENT_GB_TEXTURE_2D_NV */:
      return "GL_DEPENDENT_GB_TEXTURE_2D_NV (0x86EA)";
    case 0x86EC /* GL_DOT_PRODUCT_NV */:
      return "GL_DOT_PRODUCT_NV (0x86EC)";
    case 0x86ED /* GL_DOT_PRODUCT_DEPTH_REPLACE_NV */:
      return "GL_DOT_PRODUCT_DEPTH_REPLACE_NV (0x86ED)";
    case 0x86EE /* GL_DOT_PRODUCT_TEXTURE_2D_NV */:
      return "GL_DOT_PRODUCT_TEXTURE_2D_NV (0x86EE)";
    case 0x86F0 /* GL_DOT_PRODUCT_TEXTURE_CUBE_MAP_NV */:
      return "GL_DOT_PRODUCT_TEXTURE_CUBE_MAP_NV (0x86F0)";
    case 0x86F1 /* GL_DOT_PRODUCT_DIFFUSE_CUBE_MAP_NV */:
      return "GL_DOT_PRODUCT_DIFFUSE_CUBE_MAP_NV (0x86F1)";
    case 0x86F2 /* GL_DOT_PRODUCT_REFLECT_CUBE_MAP_NV */:
      return "GL_DOT_PRODUCT_REFLECT_CUBE_MAP_NV (0x86F2)";
    case 0x86F3 /* GL_DOT_PRODUCT_CONST_EYE_REFLECT_CUBE_MAP_NV */:
      return "GL_DOT_PRODUCT_CONST_EYE_REFLECT_CUBE_MAP_NV (0x86F3)";
    case 0x86F4 /* GL_HILO_NV */:
      return "GL_HILO_NV (0x86F4)";
    case 0x86F5 /* GL_DSDT_NV */:
      return "GL_DSDT_NV (0x86F5)";
    case 0x86F6 /* GL_DSDT_MAG_NV */:
      return "GL_DSDT_MAG_NV (0x86F6)";
    case 0x86F7 /* GL_DSDT_MAG_VIB_NV */:
      return "GL_DSDT_MAG_VIB_NV (0x86F7)";
    case 0x86F8 /* GL_HILO16_NV */:
      return "GL_HILO16_NV (0x86F8)";
    case 0x86F9 /* GL_SIGNED_HILO_NV */:
      return "GL_SIGNED_HILO_NV (0x86F9)";
    case 0x86FA /* GL_SIGNED_HILO16_NV */:
      return "GL_SIGNED_HILO16_NV (0x86FA)";
    case 0x86FB /* GL_SIGNED_RGBA_NV */:
      return "GL_SIGNED_RGBA_NV (0x86FB)";
    case 0x86FC /* GL_SIGNED_RGBA8_NV */:
      return "GL_SIGNED_RGBA8_NV (0x86FC)";
    case 0x86FE /* GL_SIGNED_RGB_NV */:
      return "GL_SIGNED_RGB_NV (0x86FE)";
    case 0x86FF /* GL_SIGNED_RGB8_NV */:
      return "GL_SIGNED_RGB8_NV (0x86FF)";
    case 0x8701 /* GL_SIGNED_LUMINANCE_NV */:
      return "GL_SIGNED_LUMINANCE_NV (0x8701)";
    case 0x8702 /* GL_SIGNED_LUMINANCE8_NV */:
      return "GL_SIGNED_LUMINANCE8_NV (0x8702)";
    case 0x8703 /* GL_SIGNED_LUMINANCE_ALPHA_NV */:
      return "GL_SIGNED_LUMINANCE_ALPHA_NV (0x8703)";
    case 0x8704 /* GL_SIGNED_LUMINANCE8_ALPHA8_NV */:
      return "GL_SIGNED_LUMINANCE8_ALPHA8_NV (0x8704)";
    case 0x8705 /* GL_SIGNED_ALPHA_NV */:
      return "GL_SIGNED_ALPHA_NV (0x8705)";
    case 0x8706 /* GL_SIGNED_ALPHA8_NV */:
      return "GL_SIGNED_ALPHA8_NV (0x8706)";
    case 0x8707 /* GL_SIGNED_INTENSITY_NV */:
      return "GL_SIGNED_INTENSITY_NV (0x8707)";
    case 0x8708 /* GL_SIGNED_INTENSITY8_NV */:
      return "GL_SIGNED_INTENSITY8_NV (0x8708)";
    case 0x8709 /* GL_DSDT8_NV */:
      return "GL_DSDT8_NV (0x8709)";
    case 0x870A /* GL_DSDT8_MAG8_NV */:
      return "GL_DSDT8_MAG8_NV (0x870A)";
    case 0x870B /* GL_DSDT8_MAG8_INTENSITY8_NV */:
      return "GL_DSDT8_MAG8_INTENSITY8_NV (0x870B)";
    case 0x870C /* GL_SIGNED_RGB_UNSIGNED_ALPHA_NV */:
      return "GL_SIGNED_RGB_UNSIGNED_ALPHA_NV (0x870C)";
    case 0x870D /* GL_SIGNED_RGB8_UNSIGNED_ALPHA8_NV */:
      return "GL_SIGNED_RGB8_UNSIGNED_ALPHA8_NV (0x870D)";
    case 0x870E /* GL_HI_SCALE_NV */:
      return "GL_HI_SCALE_NV (0x870E)";
    case 0x870F /* GL_LO_SCALE_NV */:
      return "GL_LO_SCALE_NV (0x870F)";
    case 0x8710 /* GL_DS_SCALE_NV */:
      return "GL_DS_SCALE_NV (0x8710)";
    case 0x8711 /* GL_DT_SCALE_NV */:
      return "GL_DT_SCALE_NV (0x8711)";
    case 0x8712 /* GL_MAGNITUDE_SCALE_NV */:
      return "GL_MAGNITUDE_SCALE_NV (0x8712)";
    case 0x8713 /* GL_VIBRANCE_SCALE_NV */:
      return "GL_VIBRANCE_SCALE_NV (0x8713)";
    case 0x8714 /* GL_HI_BIAS_NV */:
      return "GL_HI_BIAS_NV (0x8714)";
    case 0x8715 /* GL_LO_BIAS_NV */:
      return "GL_LO_BIAS_NV (0x8715)";
    case 0x8716 /* GL_DS_BIAS_NV */:
      return "GL_DS_BIAS_NV (0x8716)";
    case 0x8717 /* GL_DT_BIAS_NV */:
      return "GL_DT_BIAS_NV (0x8717)";
    case 0x8718 /* GL_MAGNITUDE_BIAS_NV */:
      return "GL_MAGNITUDE_BIAS_NV (0x8718)";
    case 0x8719 /* GL_VIBRANCE_BIAS_NV */:
      return "GL_VIBRANCE_BIAS_NV (0x8719)";
    case 0x871A /* GL_TEXTURE_BORDER_VALUES_NV */:
      return "GL_TEXTURE_BORDER_VALUES_NV (0x871A)";
    case 0x871B /* GL_TEXTURE_HI_SIZE_NV */:
      return "GL_TEXTURE_HI_SIZE_NV (0x871B)";
    case 0x871C /* GL_TEXTURE_LO_SIZE_NV */:
      return "GL_TEXTURE_LO_SIZE_NV (0x871C)";
    case 0x871D /* GL_TEXTURE_DS_SIZE_NV */:
      return "GL_TEXTURE_DS_SIZE_NV (0x871D)";
    case 0x871E /* GL_TEXTURE_DT_SIZE_NV */:
      return "GL_TEXTURE_DT_SIZE_NV (0x871E)";
    case 0x871F /* GL_TEXTURE_MAG_SIZE_NV */:
      return "GL_TEXTURE_MAG_SIZE_NV (0x871F)";

    /* ------------------------- GL_NV_texture_shader2 -------------------------
     */

    // case 0x86DA /* GL_UNSIGNED_INT_S8_S8_8_8_NV */: return
    // "GL_UNSIGNED_INT_S8_S8_8_8_NV (0x86DA)"; case 0x86DB /*
    // GL_UNSIGNED_INT_8_8_S8_S8_REV_NV */: return
    // "GL_UNSIGNED_INT_8_8_S8_S8_REV_NV (0x86DB)"; case 0x86DC /*
    // GL_DSDT_MAG_INTENSITY_NV */: return "GL_DSDT_MAG_INTENSITY_NV (0x86DC)";
    case 0x86EF /* GL_DOT_PRODUCT_TEXTURE_3D_NV */:
      return "GL_DOT_PRODUCT_TEXTURE_3D_NV (0x86EF)";
      // case 0x86F4 /* GL_HILO_NV */: return "GL_HILO_NV (0x86F4)";
      // case 0x86F5 /* GL_DSDT_NV */: return "GL_DSDT_NV (0x86F5)";
      // case 0x86F6 /* GL_DSDT_MAG_NV */: return "GL_DSDT_MAG_NV (0x86F6)";
      // case 0x86F7 /* GL_DSDT_MAG_VIB_NV */: return "GL_DSDT_MAG_VIB_NV
      // (0x86F7)"; case 0x86F8 /* GL_HILO16_NV */: return "GL_HILO16_NV
      // (0x86F8)"; case 0x86F9 /* GL_SIGNED_HILO_NV */: return
      // "GL_SIGNED_HILO_NV (0x86F9)"; case 0x86FA /* GL_SIGNED_HILO16_NV */:
      // return "GL_SIGNED_HILO16_NV (0x86FA)"; case 0x86FB /* GL_SIGNED_RGBA_NV
      // */: return "GL_SIGNED_RGBA_NV (0x86FB)"; case 0x86FC /*
      // GL_SIGNED_RGBA8_NV */: return "GL_SIGNED_RGBA8_NV (0x86FC)"; case 0x86FE
      // /* GL_SIGNED_RGB_NV */: return "GL_SIGNED_RGB_NV (0x86FE)"; case 0x86FF
      // /* GL_SIGNED_RGB8_NV */: return "GL_SIGNED_RGB8_NV (0x86FF)"; case
      // 0x8701 /* GL_SIGNED_LUMINANCE_NV */: return "GL_SIGNED_LUMINANCE_NV
      // (0x8701)"; case 0x8702 /* GL_SIGNED_LUMINANCE8_NV */: return
      // "GL_SIGNED_LUMINANCE8_NV (0x8702)"; case 0x8703 /*
      // GL_SIGNED_LUMINANCE_ALPHA_NV */: return "GL_SIGNED_LUMINANCE_ALPHA_NV
      // (0x8703)"; case 0x8704 /* GL_SIGNED_LUMINANCE8_ALPHA8_NV */: return
      // "GL_SIGNED_LUMINANCE8_ALPHA8_NV (0x8704)"; case 0x8705 /*
      // GL_SIGNED_ALPHA_NV */: return "GL_SIGNED_ALPHA_NV (0x8705)"; case 0x8706
      // /* GL_SIGNED_ALPHA8_NV */: return "GL_SIGNED_ALPHA8_NV (0x8706)"; case
      // 0x8707 /* GL_SIGNED_INTENSITY_NV */: return "GL_SIGNED_INTENSITY_NV
      // (0x8707)"; case 0x8708 /* GL_SIGNED_INTENSITY8_NV */: return
      // "GL_SIGNED_INTENSITY8_NV (0x8708)"; case 0x8709 /* GL_DSDT8_NV */:
      // return "GL_DSDT8_NV (0x8709)"; case 0x870A /* GL_DSDT8_MAG8_NV */:
      // return "GL_DSDT8_MAG8_NV (0x870A)"; case 0x870B /*
      // GL_DSDT8_MAG8_INTENSITY8_NV */: return "GL_DSDT8_MAG8_INTENSITY8_NV
      // (0x870B)"; case 0x870C /* GL_SIGNED_RGB_UNSIGNED_ALPHA_NV */: return
      // "GL_SIGNED_RGB_UNSIGNED_ALPHA_NV (0x870C)"; case 0x870D /*
      // GL_SIGNED_RGB8_UNSIGNED_ALPHA8_NV */: return
      // "GL_SIGNED_RGB8_UNSIGNED_ALPHA8_NV (0x870D)";

      /* ------------------------- GL_NV_texture_shader3
       * ------------------------- */

    case 0x8850 /* GL_OFFSET_PROJECTIVE_TEXTURE_2D_NV */:
      return "GL_OFFSET_PROJECTIVE_TEXTURE_2D_NV (0x8850)";
    case 0x8851 /* GL_OFFSET_PROJECTIVE_TEXTURE_2D_SCALE_NV */:
      return "GL_OFFSET_PROJECTIVE_TEXTURE_2D_SCALE_NV (0x8851)";
    case 0x8852 /* GL_OFFSET_PROJECTIVE_TEXTURE_RECTANGLE_NV */:
      return "GL_OFFSET_PROJECTIVE_TEXTURE_RECTANGLE_NV (0x8852)";
    case 0x8853 /* GL_OFFSET_PROJECTIVE_TEXTURE_RECTANGLE_SCALE_NV */:
      return "GL_OFFSET_PROJECTIVE_TEXTURE_RECTANGLE_SCALE_NV (0x8853)";
    case 0x8854 /* GL_OFFSET_HILO_TEXTURE_2D_NV */:
      return "GL_OFFSET_HILO_TEXTURE_2D_NV (0x8854)";
    case 0x8855 /* GL_OFFSET_HILO_TEXTURE_RECTANGLE_NV */:
      return "GL_OFFSET_HILO_TEXTURE_RECTANGLE_NV (0x8855)";
    case 0x8856 /* GL_OFFSET_HILO_PROJECTIVE_TEXTURE_2D_NV */:
      return "GL_OFFSET_HILO_PROJECTIVE_TEXTURE_2D_NV (0x8856)";
    case 0x8857 /* GL_OFFSET_HILO_PROJECTIVE_TEXTURE_RECTANGLE_NV */:
      return "GL_OFFSET_HILO_PROJECTIVE_TEXTURE_RECTANGLE_NV (0x8857)";
    case 0x8858 /* GL_DEPENDENT_HILO_TEXTURE_2D_NV */:
      return "GL_DEPENDENT_HILO_TEXTURE_2D_NV (0x8858)";
    case 0x8859 /* GL_DEPENDENT_RGB_TEXTURE_3D_NV */:
      return "GL_DEPENDENT_RGB_TEXTURE_3D_NV (0x8859)";
    case 0x885A /* GL_DEPENDENT_RGB_TEXTURE_CUBE_MAP_NV */:
      return "GL_DEPENDENT_RGB_TEXTURE_CUBE_MAP_NV (0x885A)";
    case 0x885B /* GL_DOT_PRODUCT_PASS_THROUGH_NV */:
      return "GL_DOT_PRODUCT_PASS_THROUGH_NV (0x885B)";
    case 0x885C /* GL_DOT_PRODUCT_TEXTURE_1D_NV */:
      return "GL_DOT_PRODUCT_TEXTURE_1D_NV (0x885C)";
    case 0x885D /* GL_DOT_PRODUCT_AFFINE_DEPTH_REPLACE_NV */:
      return "GL_DOT_PRODUCT_AFFINE_DEPTH_REPLACE_NV (0x885D)";
    case 0x885E /* GL_HILO8_NV */:
      return "GL_HILO8_NV (0x885E)";
    case 0x885F /* GL_SIGNED_HILO8_NV */:
      return "GL_SIGNED_HILO8_NV (0x885F)";
    case 0x8860 /* GL_FORCE_BLUE_TO_ONE_NV */:
      return "GL_FORCE_BLUE_TO_ONE_NV (0x8860)";

      /* ------------------------ GL_NV_transform_feedback
       * ----------------------- */

    case 0x8C77 /* GL_BACK_PRIMARY_COLOR_NV */:
      return "GL_BACK_PRIMARY_COLOR_NV (0x8C77)";
    case 0x8C78 /* GL_BACK_SECONDARY_COLOR_NV */:
      return "GL_BACK_SECONDARY_COLOR_NV (0x8C78)";
    case 0x8C79 /* GL_TEXTURE_COORD_NV */:
      return "GL_TEXTURE_COORD_NV (0x8C79)";
    case 0x8C7A /* GL_CLIP_DISTANCE_NV */:
      return "GL_CLIP_DISTANCE_NV (0x8C7A)";
    case 0x8C7B /* GL_VERTEX_ID_NV */:
      return "GL_VERTEX_ID_NV (0x8C7B)";
    case 0x8C7C /* GL_PRIMITIVE_ID_NV */:
      return "GL_PRIMITIVE_ID_NV (0x8C7C)";
    case 0x8C7D /* GL_GENERIC_ATTRIB_NV */:
      return "GL_GENERIC_ATTRIB_NV (0x8C7D)";
    case 0x8C7E /* GL_TRANSFORM_FEEDBACK_ATTRIBS_NV */:
      return "GL_TRANSFORM_FEEDBACK_ATTRIBS_NV (0x8C7E)";
    // case 0x8C7F /* GL_TRANSFORM_FEEDBACK_BUFFER_MODE_NV */: return
    // "GL_TRANSFORM_FEEDBACK_BUFFER_MODE_NV (0x8C7F)"; case 0x8C80 /*
    // GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS_NV */: return
    // "GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS_NV (0x8C80)";
    case 0x8C81 /* GL_ACTIVE_VARYINGS_NV */:
      return "GL_ACTIVE_VARYINGS_NV (0x8C81)";
    case 0x8C82 /* GL_ACTIVE_VARYING_MAX_LENGTH_NV */:
      return "GL_ACTIVE_VARYING_MAX_LENGTH_NV (0x8C82)";
    // case 0x8C83 /* GL_TRANSFORM_FEEDBACK_VARYINGS_NV */: return
    // "GL_TRANSFORM_FEEDBACK_VARYINGS_NV (0x8C83)"; case 0x8C84 /*
    // GL_TRANSFORM_FEEDBACK_BUFFER_START_NV */: return
    // "GL_TRANSFORM_FEEDBACK_BUFFER_START_NV (0x8C84)"; case 0x8C85 /*
    // GL_TRANSFORM_FEEDBACK_BUFFER_SIZE_NV */: return
    // "GL_TRANSFORM_FEEDBACK_BUFFER_SIZE_NV (0x8C85)";
    case 0x8C86 /* GL_TRANSFORM_FEEDBACK_RECORD_NV */:
      return "GL_TRANSFORM_FEEDBACK_RECORD_NV (0x8C86)";
      // case 0x8C87 /* GL_PRIMITIVES_GENERATED_NV */: return
      // "GL_PRIMITIVES_GENERATED_NV (0x8C87)"; case 0x8C88 /*
      // GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN_NV */: return
      // "GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN_NV (0x8C88)"; case 0x8C89 /*
      // GL_RASTERIZER_DISCARD_NV */: return "GL_RASTERIZER_DISCARD_NV (0x8C89)";
      // case 0x8C8A /* GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS_NV */:
      // return "GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS_NV (0x8C8A)";
      // case 0x8C8B /* GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS_NV */: return
      // "GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS_NV (0x8C8B)"; case 0x8C8C /*
      // GL_INTERLEAVED_ATTRIBS_NV */: return "GL_INTERLEAVED_ATTRIBS_NV
      // (0x8C8C)"; case 0x8C8D /* GL_SEPARATE_ATTRIBS_NV */: return
      // "GL_SEPARATE_ATTRIBS_NV (0x8C8D)"; case 0x8C8E /*
      // GL_TRANSFORM_FEEDBACK_BUFFER_NV */: return
      // "GL_TRANSFORM_FEEDBACK_BUFFER_NV (0x8C8E)"; case 0x8C8F /*
      // GL_TRANSFORM_FEEDBACK_BUFFER_BINDING_NV */: return
      // "GL_TRANSFORM_FEEDBACK_BUFFER_BINDING_NV (0x8C8F)";/*
      // ----------------------- GL_NV_transform_feedback2
      // ----------------------- */

      // case 0x8E22 /* GL_TRANSFORM_FEEDBACK_NV */: return
      // "GL_TRANSFORM_FEEDBACK_NV (0x8E22)"; case 0x8E23 /*
      // GL_TRANSFORM_FEEDBACK_BUFFER_PAUSED_NV */: return
      // "GL_TRANSFORM_FEEDBACK_BUFFER_PAUSED_NV (0x8E23)"; case 0x8E24 /*
      // GL_TRANSFORM_FEEDBACK_BUFFER_ACTIVE_NV */: return
      // "GL_TRANSFORM_FEEDBACK_BUFFER_ACTIVE_NV (0x8E24)"; case 0x8E25 /*
      // GL_TRANSFORM_FEEDBACK_BINDING_NV */: return
      // "GL_TRANSFORM_FEEDBACK_BINDING_NV (0x8E25)";

      /* -------------------------- GL_NV_vdpau_interop
       * -------------------------- */

    case 0x86EB /* GL_SURFACE_STATE_NV */:
      return "GL_SURFACE_STATE_NV (0x86EB)";
    case 0x86FD /* GL_SURFACE_REGISTERED_NV */:
      return "GL_SURFACE_REGISTERED_NV (0x86FD)";
    case 0x8700 /* GL_SURFACE_MAPPED_NV */:
      return "GL_SURFACE_MAPPED_NV (0x8700)";
    case 0x88BE /* GL_WRITE_DISCARD_NV */:
      return "GL_WRITE_DISCARD_NV (0x88BE)"; /* ------------------------
                                                GL_NV_vertex_array_range
                                                ----------------------- */

      // case 0x851D /* GL_VERTEX_ARRAY_RANGE_NV */: return
      // "GL_VERTEX_ARRAY_RANGE_NV (0x851D)"; case 0x851E /*
      // GL_VERTEX_ARRAY_RANGE_LENGTH_NV */: return
      // "GL_VERTEX_ARRAY_RANGE_LENGTH_NV (0x851E)"; case 0x851F /*
      // GL_VERTEX_ARRAY_RANGE_VALID_NV */: return
      // "GL_VERTEX_ARRAY_RANGE_VALID_NV (0x851F)"; case 0x8520 /*
      // GL_MAX_VERTEX_ARRAY_RANGE_ELEMENT_NV */: return
      // "GL_MAX_VERTEX_ARRAY_RANGE_ELEMENT_NV (0x8520)"; case 0x8521 /*
      // GL_VERTEX_ARRAY_RANGE_POINTER_NV */: return
      // "GL_VERTEX_ARRAY_RANGE_POINTER_NV (0x8521)";

      /* ----------------------- GL_NV_vertex_array_range2
       * ----------------------- */

    case 0x8533 /* GL_VERTEX_ARRAY_RANGE_WITHOUT_FLUSH_NV */:
      return "GL_VERTEX_ARRAY_RANGE_WITHOUT_FLUSH_NV (0x8533)";

      /* ------------------- GL_NV_vertex_attrib_integer_64bit
       * ------------------- */

      // case 0x140E /* GL_INT64_NV */: return "GL_INT64_NV (0x140E)";
      // case 0x140F /* GL_UNSIGNED_INT64_NV */: return "GL_UNSIGNED_INT64_NV
      // (0x140F)";

      /* ------------------- GL_NV_vertex_buffer_unified_memory
       * ------------------ */

    case 0x8F1E /* GL_VERTEX_ATTRIB_ARRAY_UNIFIED_NV */:
      return "GL_VERTEX_ATTRIB_ARRAY_UNIFIED_NV (0x8F1E)";
    case 0x8F1F /* GL_ELEMENT_ARRAY_UNIFIED_NV */:
      return "GL_ELEMENT_ARRAY_UNIFIED_NV (0x8F1F)";
    case 0x8F20 /* GL_VERTEX_ATTRIB_ARRAY_ADDRESS_NV */:
      return "GL_VERTEX_ATTRIB_ARRAY_ADDRESS_NV (0x8F20)";
    case 0x8F21 /* GL_VERTEX_ARRAY_ADDRESS_NV */:
      return "GL_VERTEX_ARRAY_ADDRESS_NV (0x8F21)";
    case 0x8F22 /* GL_NORMAL_ARRAY_ADDRESS_NV */:
      return "GL_NORMAL_ARRAY_ADDRESS_NV (0x8F22)";
    case 0x8F23 /* GL_COLOR_ARRAY_ADDRESS_NV */:
      return "GL_COLOR_ARRAY_ADDRESS_NV (0x8F23)";
    case 0x8F24 /* GL_INDEX_ARRAY_ADDRESS_NV */:
      return "GL_INDEX_ARRAY_ADDRESS_NV (0x8F24)";
    case 0x8F25 /* GL_TEXTURE_COORD_ARRAY_ADDRESS_NV */:
      return "GL_TEXTURE_COORD_ARRAY_ADDRESS_NV (0x8F25)";
    case 0x8F26 /* GL_EDGE_FLAG_ARRAY_ADDRESS_NV */:
      return "GL_EDGE_FLAG_ARRAY_ADDRESS_NV (0x8F26)";
    case 0x8F27 /* GL_SECONDARY_COLOR_ARRAY_ADDRESS_NV */:
      return "GL_SECONDARY_COLOR_ARRAY_ADDRESS_NV (0x8F27)";
    case 0x8F28 /* GL_FOG_COORD_ARRAY_ADDRESS_NV */:
      return "GL_FOG_COORD_ARRAY_ADDRESS_NV (0x8F28)";
    case 0x8F29 /* GL_ELEMENT_ARRAY_ADDRESS_NV */:
      return "GL_ELEMENT_ARRAY_ADDRESS_NV (0x8F29)";
    case 0x8F2A /* GL_VERTEX_ATTRIB_ARRAY_LENGTH_NV */:
      return "GL_VERTEX_ATTRIB_ARRAY_LENGTH_NV (0x8F2A)";
    case 0x8F2B /* GL_VERTEX_ARRAY_LENGTH_NV */:
      return "GL_VERTEX_ARRAY_LENGTH_NV (0x8F2B)";
    case 0x8F2C /* GL_NORMAL_ARRAY_LENGTH_NV */:
      return "GL_NORMAL_ARRAY_LENGTH_NV (0x8F2C)";
    case 0x8F2D /* GL_COLOR_ARRAY_LENGTH_NV */:
      return "GL_COLOR_ARRAY_LENGTH_NV (0x8F2D)";
    case 0x8F2E /* GL_INDEX_ARRAY_LENGTH_NV */:
      return "GL_INDEX_ARRAY_LENGTH_NV (0x8F2E)";
    case 0x8F2F /* GL_TEXTURE_COORD_ARRAY_LENGTH_NV */:
      return "GL_TEXTURE_COORD_ARRAY_LENGTH_NV (0x8F2F)";
    case 0x8F30 /* GL_EDGE_FLAG_ARRAY_LENGTH_NV */:
      return "GL_EDGE_FLAG_ARRAY_LENGTH_NV (0x8F30)";
    case 0x8F31 /* GL_SECONDARY_COLOR_ARRAY_LENGTH_NV */:
      return "GL_SECONDARY_COLOR_ARRAY_LENGTH_NV (0x8F31)";
    case 0x8F32 /* GL_FOG_COORD_ARRAY_LENGTH_NV */:
      return "GL_FOG_COORD_ARRAY_LENGTH_NV (0x8F32)";
    case 0x8F33 /* GL_ELEMENT_ARRAY_LENGTH_NV */:
      return "GL_ELEMENT_ARRAY_LENGTH_NV (0x8F33)";
    case 0x8F40 /* GL_DRAW_INDIRECT_UNIFIED_NV */:
      return "GL_DRAW_INDIRECT_UNIFIED_NV (0x8F40)";
    case 0x8F41 /* GL_DRAW_INDIRECT_ADDRESS_NV */:
      return "GL_DRAW_INDIRECT_ADDRESS_NV (0x8F41)";
    case 0x8F42 /* GL_DRAW_INDIRECT_LENGTH_NV */:
      return "GL_DRAW_INDIRECT_LENGTH_NV (0x8F42)";

    /* -------------------------- GL_NV_vertex_program -------------------------
     */

    // case 0x8620 /* GL_VERTEX_PROGRAM_NV */: return "GL_VERTEX_PROGRAM_NV
    // (0x8620)";
    case 0x8621 /* GL_VERTEX_STATE_PROGRAM_NV */:
      return "GL_VERTEX_STATE_PROGRAM_NV (0x8621)";
    // case 0x8623 /* GL_ATTRIB_ARRAY_SIZE_NV */: return
    // "GL_ATTRIB_ARRAY_SIZE_NV (0x8623)"; case 0x8624 /*
    // GL_ATTRIB_ARRAY_STRIDE_NV */: return "GL_ATTRIB_ARRAY_STRIDE_NV (0x8624)";
    // case 0x8625 /* GL_ATTRIB_ARRAY_TYPE_NV */: return
    // "GL_ATTRIB_ARRAY_TYPE_NV (0x8625)"; case 0x8626 /* GL_CURRENT_ATTRIB_NV
    // */: return "GL_CURRENT_ATTRIB_NV (0x8626)"; case 0x8627 /*
    // GL_PROGRAM_LENGTH_NV */: return "GL_PROGRAM_LENGTH_NV (0x8627)"; case
    // 0x8628 /* GL_PROGRAM_STRING_NV */: return "GL_PROGRAM_STRING_NV (0x8628)";
    case 0x8629 /* GL_MODELVIEW_PROJECTION_NV */:
      return "GL_MODELVIEW_PROJECTION_NV (0x8629)";
    case 0x862A /* GL_IDENTITY_NV */:
      return "GL_IDENTITY_NV (0x862A)";
    case 0x862B /* GL_INVERSE_NV */:
      return "GL_INVERSE_NV (0x862B)";
    case 0x862C /* GL_TRANSPOSE_NV */:
      return "GL_TRANSPOSE_NV (0x862C)";
    case 0x862D /* GL_INVERSE_TRANSPOSE_NV */:
      return "GL_INVERSE_TRANSPOSE_NV (0x862D)";
    // case 0x862E /* GL_MAX_TRACK_MATRIX_STACK_DEPTH_NV */: return
    // "GL_MAX_TRACK_MATRIX_STACK_DEPTH_NV (0x862E)"; case 0x862F /*
    // GL_MAX_TRACK_MATRICES_NV */: return "GL_MAX_TRACK_MATRICES_NV (0x862F)";
    case 0x8630 /* GL_MATRIX0_NV */:
      return "GL_MATRIX0_NV (0x8630)";
    case 0x8631 /* GL_MATRIX1_NV */:
      return "GL_MATRIX1_NV (0x8631)";
    case 0x8632 /* GL_MATRIX2_NV */:
      return "GL_MATRIX2_NV (0x8632)";
    case 0x8633 /* GL_MATRIX3_NV */:
      return "GL_MATRIX3_NV (0x8633)";
    case 0x8634 /* GL_MATRIX4_NV */:
      return "GL_MATRIX4_NV (0x8634)";
    case 0x8635 /* GL_MATRIX5_NV */:
      return "GL_MATRIX5_NV (0x8635)";
    case 0x8636 /* GL_MATRIX6_NV */:
      return "GL_MATRIX6_NV (0x8636)";
    case 0x8637 /* GL_MATRIX7_NV */:
      return "GL_MATRIX7_NV (0x8637)";
    // case 0x8640 /* GL_CURRENT_MATRIX_STACK_DEPTH_NV */: return
    // "GL_CURRENT_MATRIX_STACK_DEPTH_NV (0x8640)"; case 0x8641 /*
    // GL_CURRENT_MATRIX_NV */: return "GL_CURRENT_MATRIX_NV (0x8641)"; case
    // 0x8642 /* GL_VERTEX_PROGRAM_POINT_SIZE_NV */: return
    // "GL_VERTEX_PROGRAM_POINT_SIZE_NV (0x8642)"; case 0x8643 /*
    // GL_VERTEX_PROGRAM_TWO_SIDE_NV */: return "GL_VERTEX_PROGRAM_TWO_SIDE_NV
    // (0x8643)";
    case 0x8644 /* GL_PROGRAM_PARAMETER_NV */:
      return "GL_PROGRAM_PARAMETER_NV (0x8644)";
    // case 0x8645 /* GL_ATTRIB_ARRAY_POINTER_NV */: return
    // "GL_ATTRIB_ARRAY_POINTER_NV (0x8645)";
    case 0x8646 /* GL_PROGRAM_TARGET_NV */:
      return "GL_PROGRAM_TARGET_NV (0x8646)";
    case 0x8647 /* GL_PROGRAM_RESIDENT_NV */:
      return "GL_PROGRAM_RESIDENT_NV (0x8647)";
    case 0x8648 /* GL_TRACK_MATRIX_NV */:
      return "GL_TRACK_MATRIX_NV (0x8648)";
    case 0x8649 /* GL_TRACK_MATRIX_TRANSFORM_NV */:
      return "GL_TRACK_MATRIX_TRANSFORM_NV (0x8649)";
    case 0x864A /* GL_VERTEX_PROGRAM_BINDING_NV */:
      return "GL_VERTEX_PROGRAM_BINDING_NV (0x864A)";
    // case 0x864B /* GL_PROGRAM_ERROR_POSITION_NV */: return
    // "GL_PROGRAM_ERROR_POSITION_NV (0x864B)";
    case 0x8650 /* GL_VERTEX_ATTRIB_ARRAY0_NV */:
      return "GL_VERTEX_ATTRIB_ARRAY0_NV (0x8650)";
    case 0x8651 /* GL_VERTEX_ATTRIB_ARRAY1_NV */:
      return "GL_VERTEX_ATTRIB_ARRAY1_NV (0x8651)";
    case 0x8652 /* GL_VERTEX_ATTRIB_ARRAY2_NV */:
      return "GL_VERTEX_ATTRIB_ARRAY2_NV (0x8652)";
    case 0x8653 /* GL_VERTEX_ATTRIB_ARRAY3_NV */:
      return "GL_VERTEX_ATTRIB_ARRAY3_NV (0x8653)";
    case 0x8654 /* GL_VERTEX_ATTRIB_ARRAY4_NV */:
      return "GL_VERTEX_ATTRIB_ARRAY4_NV (0x8654)";
    case 0x8655 /* GL_VERTEX_ATTRIB_ARRAY5_NV */:
      return "GL_VERTEX_ATTRIB_ARRAY5_NV (0x8655)";
    case 0x8656 /* GL_VERTEX_ATTRIB_ARRAY6_NV */:
      return "GL_VERTEX_ATTRIB_ARRAY6_NV (0x8656)";
    case 0x8657 /* GL_VERTEX_ATTRIB_ARRAY7_NV */:
      return "GL_VERTEX_ATTRIB_ARRAY7_NV (0x8657)";
    case 0x8658 /* GL_VERTEX_ATTRIB_ARRAY8_NV */:
      return "GL_VERTEX_ATTRIB_ARRAY8_NV (0x8658)";
    case 0x8659 /* GL_VERTEX_ATTRIB_ARRAY9_NV */:
      return "GL_VERTEX_ATTRIB_ARRAY9_NV (0x8659)";
    case 0x865A /* GL_VERTEX_ATTRIB_ARRAY10_NV */:
      return "GL_VERTEX_ATTRIB_ARRAY10_NV (0x865A)";
    case 0x865B /* GL_VERTEX_ATTRIB_ARRAY11_NV */:
      return "GL_VERTEX_ATTRIB_ARRAY11_NV (0x865B)";
    case 0x865C /* GL_VERTEX_ATTRIB_ARRAY12_NV */:
      return "GL_VERTEX_ATTRIB_ARRAY12_NV (0x865C)";
    case 0x865D /* GL_VERTEX_ATTRIB_ARRAY13_NV */:
      return "GL_VERTEX_ATTRIB_ARRAY13_NV (0x865D)";
    case 0x865E /* GL_VERTEX_ATTRIB_ARRAY14_NV */:
      return "GL_VERTEX_ATTRIB_ARRAY14_NV (0x865E)";
    case 0x865F /* GL_VERTEX_ATTRIB_ARRAY15_NV */:
      return "GL_VERTEX_ATTRIB_ARRAY15_NV (0x865F)";
    case 0x8660 /* GL_MAP1_VERTEX_ATTRIB0_4_NV */:
      return "GL_MAP1_VERTEX_ATTRIB0_4_NV (0x8660)";
    case 0x8661 /* GL_MAP1_VERTEX_ATTRIB1_4_NV */:
      return "GL_MAP1_VERTEX_ATTRIB1_4_NV (0x8661)";
    case 0x8662 /* GL_MAP1_VERTEX_ATTRIB2_4_NV */:
      return "GL_MAP1_VERTEX_ATTRIB2_4_NV (0x8662)";
    case 0x8663 /* GL_MAP1_VERTEX_ATTRIB3_4_NV */:
      return "GL_MAP1_VERTEX_ATTRIB3_4_NV (0x8663)";
    case 0x8664 /* GL_MAP1_VERTEX_ATTRIB4_4_NV */:
      return "GL_MAP1_VERTEX_ATTRIB4_4_NV (0x8664)";
    case 0x8665 /* GL_MAP1_VERTEX_ATTRIB5_4_NV */:
      return "GL_MAP1_VERTEX_ATTRIB5_4_NV (0x8665)";
    case 0x8666 /* GL_MAP1_VERTEX_ATTRIB6_4_NV */:
      return "GL_MAP1_VERTEX_ATTRIB6_4_NV (0x8666)";
    case 0x8667 /* GL_MAP1_VERTEX_ATTRIB7_4_NV */:
      return "GL_MAP1_VERTEX_ATTRIB7_4_NV (0x8667)";
    case 0x8668 /* GL_MAP1_VERTEX_ATTRIB8_4_NV */:
      return "GL_MAP1_VERTEX_ATTRIB8_4_NV (0x8668)";
    case 0x8669 /* GL_MAP1_VERTEX_ATTRIB9_4_NV */:
      return "GL_MAP1_VERTEX_ATTRIB9_4_NV (0x8669)";
    case 0x866A /* GL_MAP1_VERTEX_ATTRIB10_4_NV */:
      return "GL_MAP1_VERTEX_ATTRIB10_4_NV (0x866A)";
    case 0x866B /* GL_MAP1_VERTEX_ATTRIB11_4_NV */:
      return "GL_MAP1_VERTEX_ATTRIB11_4_NV (0x866B)";
    case 0x866C /* GL_MAP1_VERTEX_ATTRIB12_4_NV */:
      return "GL_MAP1_VERTEX_ATTRIB12_4_NV (0x866C)";
    case 0x866D /* GL_MAP1_VERTEX_ATTRIB13_4_NV */:
      return "GL_MAP1_VERTEX_ATTRIB13_4_NV (0x866D)";
    case 0x866E /* GL_MAP1_VERTEX_ATTRIB14_4_NV */:
      return "GL_MAP1_VERTEX_ATTRIB14_4_NV (0x866E)";
    case 0x866F /* GL_MAP1_VERTEX_ATTRIB15_4_NV */:
      return "GL_MAP1_VERTEX_ATTRIB15_4_NV (0x866F)";
    case 0x8670 /* GL_MAP2_VERTEX_ATTRIB0_4_NV */:
      return "GL_MAP2_VERTEX_ATTRIB0_4_NV (0x8670)";
    case 0x8671 /* GL_MAP2_VERTEX_ATTRIB1_4_NV */:
      return "GL_MAP2_VERTEX_ATTRIB1_4_NV (0x8671)";
    case 0x8672 /* GL_MAP2_VERTEX_ATTRIB2_4_NV */:
      return "GL_MAP2_VERTEX_ATTRIB2_4_NV (0x8672)";
    case 0x8673 /* GL_MAP2_VERTEX_ATTRIB3_4_NV */:
      return "GL_MAP2_VERTEX_ATTRIB3_4_NV (0x8673)";
    case 0x8674 /* GL_MAP2_VERTEX_ATTRIB4_4_NV */:
      return "GL_MAP2_VERTEX_ATTRIB4_4_NV (0x8674)";
    case 0x8675 /* GL_MAP2_VERTEX_ATTRIB5_4_NV */:
      return "GL_MAP2_VERTEX_ATTRIB5_4_NV (0x8675)";
    case 0x8676 /* GL_MAP2_VERTEX_ATTRIB6_4_NV */:
      return "GL_MAP2_VERTEX_ATTRIB6_4_NV (0x8676)";
    // case 0x8677 /* GL_MAP2_VERTEX_ATTRIB7_4_NV */: return
    // "GL_MAP2_VERTEX_ATTRIB7_4_NV (0x8677)";
    case 0x8678 /* GL_MAP2_VERTEX_ATTRIB8_4_NV */:
      return "GL_MAP2_VERTEX_ATTRIB8_4_NV (0x8678)";
    case 0x8679 /* GL_MAP2_VERTEX_ATTRIB9_4_NV */:
      return "GL_MAP2_VERTEX_ATTRIB9_4_NV (0x8679)";
    case 0x867A /* GL_MAP2_VERTEX_ATTRIB10_4_NV */:
      return "GL_MAP2_VERTEX_ATTRIB10_4_NV (0x867A)";
    case 0x867B /* GL_MAP2_VERTEX_ATTRIB11_4_NV */:
      return "GL_MAP2_VERTEX_ATTRIB11_4_NV (0x867B)";
    case 0x867C /* GL_MAP2_VERTEX_ATTRIB12_4_NV */:
      return "GL_MAP2_VERTEX_ATTRIB12_4_NV (0x867C)";
    case 0x867D /* GL_MAP2_VERTEX_ATTRIB13_4_NV */:
      return "GL_MAP2_VERTEX_ATTRIB13_4_NV (0x867D)";
    case 0x867E /* GL_MAP2_VERTEX_ATTRIB14_4_NV */:
      return "GL_MAP2_VERTEX_ATTRIB14_4_NV (0x867E)";
    case 0x867F /* GL_MAP2_VERTEX_ATTRIB15_4_NV */:
      return "GL_MAP2_VERTEX_ATTRIB15_4_NV (0x867F)";

      /* ------------------------ GL_NV_vertex_program1_1
       * ------------------------ */

      /* ------------------------- GL_NV_vertex_program2
       * ------------------------- */

      /* ---------------------- GL_NV_vertex_program2_option
       * --------------------- */

      // case 0x88F5 /* GL_MAX_PROGRAM_CALL_DEPTH_NV */: return
      // "GL_MAX_PROGRAM_CALL_DEPTH_NV (0x88F5)";

      /* ------------------------- GL_NV_vertex_program3
       * ------------------------- */
      /* ------------------------- GL_NV_vertex_program4
       * ------------------------- */

      // case 0x88FD /* GL_VERTEX_ATTRIB_ARRAY_INTEGER_NV */: return
      // "GL_VERTEX_ATTRIB_ARRAY_INTEGER_NV (0x88FD)";

      /* -------------------------- GL_NV_video_capture
       * -------------------------- */

    case 0x9020 /* GL_VIDEO_BUFFER_NV */:
      return "GL_VIDEO_BUFFER_NV (0x9020)";
    case 0x9021 /* GL_VIDEO_BUFFER_BINDING_NV */:
      return "GL_VIDEO_BUFFER_BINDING_NV (0x9021)";
    case 0x9022 /* GL_FIELD_UPPER_NV */:
      return "GL_FIELD_UPPER_NV (0x9022)";
    case 0x9023 /* GL_FIELD_LOWER_NV */:
      return "GL_FIELD_LOWER_NV (0x9023)";
    case 0x9024 /* GL_NUM_VIDEO_CAPTURE_STREAMS_NV */:
      return "GL_NUM_VIDEO_CAPTURE_STREAMS_NV (0x9024)";
    case 0x9025 /* GL_NEXT_VIDEO_CAPTURE_BUFFER_STATUS_NV */:
      return "GL_NEXT_VIDEO_CAPTURE_BUFFER_STATUS_NV (0x9025)";
    case 0x9026 /* GL_VIDEO_CAPTURE_TO_422_SUPPORTED_NV */:
      return "GL_VIDEO_CAPTURE_TO_422_SUPPORTED_NV (0x9026)";
    case 0x9027 /* GL_LAST_VIDEO_CAPTURE_STATUS_NV */:
      return "GL_LAST_VIDEO_CAPTURE_STATUS_NV (0x9027)";
    case 0x9028 /* GL_VIDEO_BUFFER_PITCH_NV */:
      return "GL_VIDEO_BUFFER_PITCH_NV (0x9028)";
    case 0x9029 /* GL_VIDEO_COLOR_CONVERSION_MATRIX_NV */:
      return "GL_VIDEO_COLOR_CONVERSION_MATRIX_NV (0x9029)";
    case 0x902A /* GL_VIDEO_COLOR_CONVERSION_MAX_NV */:
      return "GL_VIDEO_COLOR_CONVERSION_MAX_NV (0x902A)";
    case 0x902B /* GL_VIDEO_COLOR_CONVERSION_MIN_NV */:
      return "GL_VIDEO_COLOR_CONVERSION_MIN_NV (0x902B)";
    case 0x902C /* GL_VIDEO_COLOR_CONVERSION_OFFSET_NV */:
      return "GL_VIDEO_COLOR_CONVERSION_OFFSET_NV (0x902C)";
    case 0x902D /* GL_VIDEO_BUFFER_INTERNAL_FORMAT_NV */:
      return "GL_VIDEO_BUFFER_INTERNAL_FORMAT_NV (0x902D)";
    case 0x902E /* GL_PARTIAL_SUCCESS_NV */:
      return "GL_PARTIAL_SUCCESS_NV (0x902E)";
    case 0x902F /* GL_SUCCESS_NV */:
      return "GL_SUCCESS_NV (0x902F)";
    case 0x9030 /* GL_FAILURE_NV */:
      return "GL_FAILURE_NV (0x9030)";
    case 0x9031 /* GL_YCBYCR8_422_NV */:
      return "GL_YCBYCR8_422_NV (0x9031)";
    case 0x9032 /* GL_YCBAYCR8A_4224_NV */:
      return "GL_YCBAYCR8A_4224_NV (0x9032)";
    case 0x9033 /* GL_Z6Y10Z6CB10Z6Y10Z6CR10_422_NV */:
      return "GL_Z6Y10Z6CB10Z6Y10Z6CR10_422_NV (0x9033)";
    case 0x9034 /* GL_Z6Y10Z6CB10Z6A10Z6Y10Z6CR10Z6A10_4224_NV */:
      return "GL_Z6Y10Z6CB10Z6A10Z6Y10Z6CR10Z6A10_4224_NV (0x9034)";
    case 0x9035 /* GL_Z4Y12Z4CB12Z4Y12Z4CR12_422_NV */:
      return "GL_Z4Y12Z4CB12Z4Y12Z4CR12_422_NV (0x9035)";
    case 0x9036 /* GL_Z4Y12Z4CB12Z4A12Z4Y12Z4CR12Z4A12_4224_NV */:
      return "GL_Z4Y12Z4CB12Z4A12Z4Y12Z4CR12Z4A12_4224_NV (0x9036)";
    case 0x9037 /* GL_Z4Y12Z4CB12Z4CR12_444_NV */:
      return "GL_Z4Y12Z4CB12Z4CR12_444_NV (0x9037)";
    case 0x9038 /* GL_VIDEO_CAPTURE_FRAME_WIDTH_NV */:
      return "GL_VIDEO_CAPTURE_FRAME_WIDTH_NV (0x9038)";
    case 0x9039 /* GL_VIDEO_CAPTURE_FRAME_HEIGHT_NV */:
      return "GL_VIDEO_CAPTURE_FRAME_HEIGHT_NV (0x9039)";
    case 0x903A /* GL_VIDEO_CAPTURE_FIELD_UPPER_HEIGHT_NV */:
      return "GL_VIDEO_CAPTURE_FIELD_UPPER_HEIGHT_NV (0x903A)";
    case 0x903B /* GL_VIDEO_CAPTURE_FIELD_LOWER_HEIGHT_NV */:
      return "GL_VIDEO_CAPTURE_FIELD_LOWER_HEIGHT_NV (0x903B)";
    case 0x903C /* GL_VIDEO_CAPTURE_SURFACE_ORIGIN_NV */:
      return "GL_VIDEO_CAPTURE_SURFACE_ORIGIN_NV (0x903C)";

      /* ------------------------ GL_OES_byte_coordinates
       * ------------------------ */

      // case 0x1400 /* GL_BYTE */: return "GL_BYTE (0x1400)";

      /* ------------------- GL_OES_compressed_paletted_texture
       * ------------------ */

    case 0x8B90 /* GL_PALETTE4_RGB8_OES */:
      return "GL_PALETTE4_RGB8_OES (0x8B90)";
    case 0x8B91 /* GL_PALETTE4_RGBA8_OES */:
      return "GL_PALETTE4_RGBA8_OES (0x8B91)";
    case 0x8B92 /* GL_PALETTE4_R5_G6_B5_OES */:
      return "GL_PALETTE4_R5_G6_B5_OES (0x8B92)";
    case 0x8B93 /* GL_PALETTE4_RGBA4_OES */:
      return "GL_PALETTE4_RGBA4_OES (0x8B93)";
    case 0x8B94 /* GL_PALETTE4_RGB5_A1_OES */:
      return "GL_PALETTE4_RGB5_A1_OES (0x8B94)";
    case 0x8B95 /* GL_PALETTE8_RGB8_OES */:
      return "GL_PALETTE8_RGB8_OES (0x8B95)";
    case 0x8B96 /* GL_PALETTE8_RGBA8_OES */:
      return "GL_PALETTE8_RGBA8_OES (0x8B96)";
    case 0x8B97 /* GL_PALETTE8_R5_G6_B5_OES */:
      return "GL_PALETTE8_R5_G6_B5_OES (0x8B97)";
    case 0x8B98 /* GL_PALETTE8_RGBA4_OES */:
      return "GL_PALETTE8_RGBA4_OES (0x8B98)";
    case 0x8B99 /* GL_PALETTE8_RGB5_A1_OES */:
      return "GL_PALETTE8_RGB5_A1_OES (0x8B99)";

      /* --------------------------- GL_OES_read_format
       * -------------------------- */

      // case 0x8B9A /* GL_IMPLEMENTATION_COLOR_READ_TYPE_OES */: return
      // "GL_IMPLEMENTATION_COLOR_READ_TYPE_OES (0x8B9A)"; case 0x8B9B /*
      // GL_IMPLEMENTATION_COLOR_READ_FORMAT_OES */: return
      // "GL_IMPLEMENTATION_COLOR_READ_FORMAT_OES (0x8B9B)";

      /* ------------------------ GL_OES_single_precision
       * ------------------------ */

      /* ---------------------------- GL_OML_interlace
       * --------------------------- */

    case 0x8980 /* GL_INTERLACE_OML */:
      return "GL_INTERLACE_OML (0x8980)";
    case 0x8981 /* GL_INTERLACE_READ_OML */:
      return "GL_INTERLACE_READ_OML (0x8981)";

      /* ---------------------------- GL_OML_resample
       * ---------------------------- */

    case 0x8984 /* GL_PACK_RESAMPLE_OML */:
      return "GL_PACK_RESAMPLE_OML (0x8984)";
    case 0x8985 /* GL_UNPACK_RESAMPLE_OML */:
      return "GL_UNPACK_RESAMPLE_OML (0x8985)";
    case 0x8986 /* GL_RESAMPLE_REPLICATE_OML */:
      return "GL_RESAMPLE_REPLICATE_OML (0x8986)";
    case 0x8987 /* GL_RESAMPLE_ZERO_FILL_OML */:
      return "GL_RESAMPLE_ZERO_FILL_OML (0x8987)";
    case 0x8988 /* GL_RESAMPLE_AVERAGE_OML */:
      return "GL_RESAMPLE_AVERAGE_OML (0x8988)";
    case 0x8989 /* GL_RESAMPLE_DECIMATE_OML */:
      return "GL_RESAMPLE_DECIMATE_OML (0x8989)";

      /* ---------------------------- GL_OML_subsample
       * --------------------------- */

    case 0x8982 /* GL_FORMAT_SUBSAMPLE_24_24_OML */:
      return "GL_FORMAT_SUBSAMPLE_24_24_OML (0x8982)";
    case 0x8983 /* GL_FORMAT_SUBSAMPLE_244_244_OML */:
      return "GL_FORMAT_SUBSAMPLE_244_244_OML (0x8983)";

    /* --------------------------- GL_PGI_misc_hints ---------------------------
     *//* -------------------------- GL_PGI_vertex_hints -------------------------- */

    // case 0x00000004 /* GL_VERTEX23_BIT_PGI */: return "GL_VERTEX23_BIT_PGI
    // (0x00000004)"; case 0x00000008 /* GL_VERTEX4_BIT_PGI */: return
    // "GL_VERTEX4_BIT_PGI (0x00000008)"; case 0x00010000 /* GL_COLOR3_BIT_PGI
    // */: return "GL_COLOR3_BIT_PGI (0x00010000)"; case 0x00020000 /*
    // GL_COLOR4_BIT_PGI */: return "GL_COLOR4_BIT_PGI (0x00020000)"; case
    // 0x00040000 /* GL_EDGEFLAG_BIT_PGI */: return "GL_EDGEFLAG_BIT_PGI
    // (0x00040000)"; case 0x00080000 /* GL_INDEX_BIT_PGI */: return
    // "GL_INDEX_BIT_PGI (0x00080000)"; case 0x00100000 /* GL_MAT_AMBIENT_BIT_PGI
    // */: return "GL_MAT_AMBIENT_BIT_PGI (0x00100000)";

    // case 0x00200000 /* GL_MAT_AMBIENT_AND_DIFFUSE_BIT_PGI */: return
    // "GL_MAT_AMBIENT_AND_DIFFUSE_BIT_PGI (0x00200000)"; case 0x00400000 /*
    // GL_MAT_DIFFUSE_BIT_PGI */: return "GL_MAT_DIFFUSE_BIT_PGI (0x00400000)";
    // case 0x00800000 /* GL_MAT_EMISSION_BIT_PGI */: return
    // "GL_MAT_EMISSION_BIT_PGI (0x00800000)"; case 0x01000000 /*
    // GL_MAT_COLOR_INDEXES_BIT_PGI */: return "GL_MAT_COLOR_INDEXES_BIT_PGI
    // (0x01000000)"; case 0x02000000 /* GL_MAT_SHININESS_BIT_PGI */: return
    // "GL_MAT_SHININESS_BIT_PGI (0x02000000)"; case 0x04000000 /*
    // GL_MAT_SPECULAR_BIT_PGI */: return "GL_MAT_SPECULAR_BIT_PGI (0x04000000)";
    // case 0x08000000 /* GL_NORMAL_BIT_PGI */: return "GL_NORMAL_BIT_PGI
    // (0x08000000)"; case 0x10000000 /* GL_TEXCOORD1_BIT_PGI */: return
    // "GL_TEXCOORD1_BIT_PGI (0x10000000)"; case 0x20000000 /*
    // GL_TEXCOORD2_BIT_PGI */: return "GL_TEXCOORD2_BIT_PGI (0x20000000)";
    case 0x40000000 /* GL_TEXCOORD3_BIT_PGI */:
      return "GL_TEXCOORD3_BIT_PGI (0x40000000)";
    case 0x80000000 /* GL_TEXCOORD4_BIT_PGI */:
      return "GL_TEXCOORD4_BIT_PGI (0x80000000)";

      /* ------------------------- GL_REGAL_error_string
       * ------------------------- */

      /* ------------------------ GL_REGAL_extension_query
       * ----------------------- */

      /* ------------------------------ GL_REGAL_log
       * ----------------------------- */

    case 0x9319 /* GL_LOG_ERROR_REGAL */:
      return "GL_LOG_ERROR_REGAL (0x9319)";
    case 0x931A /* GL_LOG_WARNING_REGAL */:
      return "GL_LOG_WARNING_REGAL (0x931A)";
    case 0x931B /* GL_LOG_INFO_REGAL */:
      return "GL_LOG_INFO_REGAL (0x931B)";
    case 0x931C /* GL_LOG_APP_REGAL */:
      return "GL_LOG_APP_REGAL (0x931C)";
    case 0x931D /* GL_LOG_DRIVER_REGAL */:
      return "GL_LOG_DRIVER_REGAL (0x931D)";
    case 0x931E /* GL_LOG_INTERNAL_REGAL */:
      return "GL_LOG_INTERNAL_REGAL (0x931E)";
    case 0x931F /* GL_LOG_DEBUG_REGAL */:
      return "GL_LOG_DEBUG_REGAL (0x931F)";
    case 0x9320 /* GL_LOG_STATUS_REGAL */:
      return "GL_LOG_STATUS_REGAL (0x9320)";
    case 0x9321 /* GL_LOG_HTTP_REGAL */:
      return "GL_LOG_HTTP_REGAL (0x9321)";

      /* ----------------------- GL_REND_screen_coordinates
       * ---------------------- */

    case 0x8490 /* GL_SCREEN_COORDINATES_REND */:
      return "GL_SCREEN_COORDINATES_REND (0x8490)";
    case 0x8491 /* GL_INVERTED_SCREEN_W_REND */:
      return "GL_INVERTED_SCREEN_W_REND (0x8491)";

      /* ------------------------------- GL_S3_s3tc
       * ------------------------------ */

    case 0x83A0 /* GL_RGB_S3TC */:
      return "GL_RGB_S3TC (0x83A0)";
    case 0x83A1 /* GL_RGB4_S3TC */:
      return "GL_RGB4_S3TC (0x83A1)";
    case 0x83A2 /* GL_RGBA_S3TC */:
      return "GL_RGBA_S3TC (0x83A2)";
    case 0x83A3 /* GL_RGBA4_S3TC */:
      return "GL_RGBA4_S3TC (0x83A3)";
    case 0x83A4 /* GL_RGBA_DXT5_S3TC */:
      return "GL_RGBA_DXT5_S3TC (0x83A4)";
    case 0x83A5 /* GL_RGBA4_DXT5_S3TC */:
      return "GL_RGBA4_DXT5_S3TC (0x83A5)";

      /* -------------------------- GL_SGIS_color_range
       * -------------------------- */

    case 0x85A5 /* GL_EXTENDED_RANGE_SGIS */:
      return "GL_EXTENDED_RANGE_SGIS (0x85A5)";
    case 0x85A6 /* GL_MIN_RED_SGIS */:
      return "GL_MIN_RED_SGIS (0x85A6)";
    case 0x85A7 /* GL_MAX_RED_SGIS */:
      return "GL_MAX_RED_SGIS (0x85A7)";
    case 0x85A8 /* GL_MIN_GREEN_SGIS */:
      return "GL_MIN_GREEN_SGIS (0x85A8)";
    case 0x85A9 /* GL_MAX_GREEN_SGIS */:
      return "GL_MAX_GREEN_SGIS (0x85A9)";
    case 0x85AA /* GL_MIN_BLUE_SGIS */:
      return "GL_MIN_BLUE_SGIS (0x85AA)";
    case 0x85AB /* GL_MAX_BLUE_SGIS */:
      return "GL_MAX_BLUE_SGIS (0x85AB)";
    case 0x85AC /* GL_MIN_ALPHA_SGIS */:
      return "GL_MIN_ALPHA_SGIS (0x85AC)";
    case 0x85AD /* GL_MAX_ALPHA_SGIS */:
      return "GL_MAX_ALPHA_SGIS (0x85AD)";

      /* ------------------------- GL_SGIS_detail_texture
       * ------------------------ */

      /* -------------------------- GL_SGIS_fog_function
       * ------------------------- */

      /* ------------------------ GL_SGIS_generate_mipmap
       * ------------------------ */

      // case 0x8191 /* GL_GENERATE_MIPMAP_SGIS */: return
      // "GL_GENERATE_MIPMAP_SGIS (0x8191)"; case 0x8192 /*
      // GL_GENERATE_MIPMAP_HINT_SGIS */: return "GL_GENERATE_MIPMAP_HINT_SGIS
      // (0x8192)";

      /* -------------------------- GL_SGIS_multisample
       * -------------------------- */

      // case 0x809D /* GL_MULTISAMPLE_SGIS */: return "GL_MULTISAMPLE_SGIS
      // (0x809D)"; case 0x809E /* GL_SAMPLE_ALPHA_TO_MASK_SGIS */: return
      // "GL_SAMPLE_ALPHA_TO_MASK_SGIS (0x809E)"; case 0x809F /*
      // GL_SAMPLE_ALPHA_TO_ONE_SGIS */: return "GL_SAMPLE_ALPHA_TO_ONE_SGIS
      // (0x809F)"; case 0x80A0 /* GL_SAMPLE_MASK_SGIS */: return
      // "GL_SAMPLE_MASK_SGIS (0x80A0)"; case 0x80A1 /* GL_1PASS_SGIS */: return
      // "GL_1PASS_SGIS (0x80A1)"; case 0x80A2 /* GL_2PASS_0_SGIS */: return
      // "GL_2PASS_0_SGIS (0x80A2)"; case 0x80A3 /* GL_2PASS_1_SGIS */: return
      // "GL_2PASS_1_SGIS (0x80A3)"; case 0x80A4 /* GL_4PASS_0_SGIS */: return
      // "GL_4PASS_0_SGIS (0x80A4)"; case 0x80A5 /* GL_4PASS_1_SGIS */: return
      // "GL_4PASS_1_SGIS (0x80A5)"; case 0x80A6 /* GL_4PASS_2_SGIS */: return
      // "GL_4PASS_2_SGIS (0x80A6)"; case 0x80A7 /* GL_4PASS_3_SGIS */: return
      // "GL_4PASS_3_SGIS (0x80A7)"; case 0x80A8 /* GL_SAMPLE_BUFFERS_SGIS */:
      // return "GL_SAMPLE_BUFFERS_SGIS (0x80A8)"; case 0x80A9 /* GL_SAMPLES_SGIS
      // */: return "GL_SAMPLES_SGIS (0x80A9)"; case 0x80AA /*
      // GL_SAMPLE_MASK_VALUE_SGIS */: return "GL_SAMPLE_MASK_VALUE_SGIS
      // (0x80AA)"; case 0x80AB /* GL_SAMPLE_MASK_INVERT_SGIS */: return
      // "GL_SAMPLE_MASK_INVERT_SGIS (0x80AB)"; case 0x80AC /*
      // GL_SAMPLE_PATTERN_SGIS */: return "GL_SAMPLE_PATTERN_SGIS (0x80AC)";

      /* ------------------------- GL_SGIS_pixel_texture
       * ------------------------- */

      /* ----------------------- GL_SGIS_point_line_texgen
       * ----------------------- */

    case 0x81F0 /* GL_EYE_DISTANCE_TO_POINT_SGIS */:
      return "GL_EYE_DISTANCE_TO_POINT_SGIS (0x81F0)";
    case 0x81F1 /* GL_OBJECT_DISTANCE_TO_POINT_SGIS */:
      return "GL_OBJECT_DISTANCE_TO_POINT_SGIS (0x81F1)";
    case 0x81F2 /* GL_EYE_DISTANCE_TO_LINE_SGIS */:
      return "GL_EYE_DISTANCE_TO_LINE_SGIS (0x81F2)";
    case 0x81F3 /* GL_OBJECT_DISTANCE_TO_LINE_SGIS */:
      return "GL_OBJECT_DISTANCE_TO_LINE_SGIS (0x81F3)";
    case 0x81F4 /* GL_EYE_POINT_SGIS */:
      return "GL_EYE_POINT_SGIS (0x81F4)";
    case 0x81F5 /* GL_OBJECT_POINT_SGIS */:
      return "GL_OBJECT_POINT_SGIS (0x81F5)";
    case 0x81F6 /* GL_EYE_LINE_SGIS */:
      return "GL_EYE_LINE_SGIS (0x81F6)";
    case 0x81F7 /* GL_OBJECT_LINE_SGIS */:
      return "GL_OBJECT_LINE_SGIS (0x81F7)";

      /* ------------------------ GL_SGIS_sharpen_texture
       * ------------------------ */

      /* --------------------------- GL_SGIS_texture4D
       * --------------------------- */

      /* ---------------------- GL_SGIS_texture_border_clamp
       * --------------------- */

      // case 0x812D /* GL_CLAMP_TO_BORDER_SGIS */: return
      // "GL_CLAMP_TO_BORDER_SGIS (0x812D)";

      /* ----------------------- GL_SGIS_texture_edge_clamp
       * ---------------------- */

      // case 0x812F /* GL_CLAMP_TO_EDGE_SGIS */: return "GL_CLAMP_TO_EDGE_SGIS
      // (0x812F)";

      /* ------------------------ GL_SGIS_texture_filter4
       * ------------------------ */

      /* -------------------------- GL_SGIS_texture_lod
       * -------------------------- */

      // case 0x813A /* GL_TEXTURE_MIN_LOD_SGIS */: return
      // "GL_TEXTURE_MIN_LOD_SGIS (0x813A)"; case 0x813B /*
      // GL_TEXTURE_MAX_LOD_SGIS */: return "GL_TEXTURE_MAX_LOD_SGIS (0x813B)";
      // case 0x813C /* GL_TEXTURE_BASE_LEVEL_SGIS */: return
      // "GL_TEXTURE_BASE_LEVEL_SGIS (0x813C)"; case 0x813D /*
      // GL_TEXTURE_MAX_LEVEL_SGIS */: return "GL_TEXTURE_MAX_LEVEL_SGIS
      // (0x813D)";

      /* ------------------------- GL_SGIS_texture_select
       * ------------------------ */

      /* ----------------------------- GL_SGIX_async
       * ----------------------------- */

    case 0x8329 /* GL_ASYNC_MARKER_SGIX */:
      return "GL_ASYNC_MARKER_SGIX (0x8329)";

      /* ------------------------ GL_SGIX_async_histogram
       * ------------------------ */

    case 0x832C /* GL_ASYNC_HISTOGRAM_SGIX */:
      return "GL_ASYNC_HISTOGRAM_SGIX (0x832C)";
    case 0x832D /* GL_MAX_ASYNC_HISTOGRAM_SGIX */:
      return "GL_MAX_ASYNC_HISTOGRAM_SGIX (0x832D)";

      /* -------------------------- GL_SGIX_async_pixel
       * -------------------------- */

    case 0x835C /* GL_ASYNC_TEX_IMAGE_SGIX */:
      return "GL_ASYNC_TEX_IMAGE_SGIX (0x835C)";
    case 0x835D /* GL_ASYNC_DRAW_PIXELS_SGIX */:
      return "GL_ASYNC_DRAW_PIXELS_SGIX (0x835D)";
    case 0x835E /* GL_ASYNC_READ_PIXELS_SGIX */:
      return "GL_ASYNC_READ_PIXELS_SGIX (0x835E)";
    case 0x835F /* GL_MAX_ASYNC_TEX_IMAGE_SGIX */:
      return "GL_MAX_ASYNC_TEX_IMAGE_SGIX (0x835F)";
    case 0x8360 /* GL_MAX_ASYNC_DRAW_PIXELS_SGIX */:
      return "GL_MAX_ASYNC_DRAW_PIXELS_SGIX (0x8360)";
    case 0x8361 /* GL_MAX_ASYNC_READ_PIXELS_SGIX */:
      return "GL_MAX_ASYNC_READ_PIXELS_SGIX (0x8361)";

      /* ----------------------- GL_SGIX_blend_alpha_minmax
       * ---------------------- */

    case 0x8320 /* GL_ALPHA_MIN_SGIX */:
      return "GL_ALPHA_MIN_SGIX (0x8320)";
    case 0x8321 /* GL_ALPHA_MAX_SGIX */:
      return "GL_ALPHA_MAX_SGIX (0x8321)";

      /* ---------------------------- GL_SGIX_clipmap
       * ---------------------------- */

      /* ---------------------- GL_SGIX_convolution_accuracy
       * --------------------- */

    case 0x8316 /* GL_CONVOLUTION_HINT_SGIX */:
      return "GL_CONVOLUTION_HINT_SGIX (0x8316)";

      /* ------------------------- GL_SGIX_depth_texture
       * ------------------------- */

      // case 0x81A5 /* GL_DEPTH_COMPONENT16_SGIX */: return
      // "GL_DEPTH_COMPONENT16_SGIX (0x81A5)"; case 0x81A6 /*
      // GL_DEPTH_COMPONENT24_SGIX */: return "GL_DEPTH_COMPONENT24_SGIX
      // (0x81A6)"; case 0x81A7 /* GL_DEPTH_COMPONENT32_SGIX */: return
      // "GL_DEPTH_COMPONENT32_SGIX (0x81A7)";

      /* -------------------------- GL_SGIX_flush_raster
       * ------------------------- */

      /* --------------------------- GL_SGIX_fog_offset
       * -------------------------- */

    case 0x8198 /* GL_FOG_OFFSET_SGIX */:
      return "GL_FOG_OFFSET_SGIX (0x8198)";
    case 0x8199 /* GL_FOG_OFFSET_VALUE_SGIX */:
      return "GL_FOG_OFFSET_VALUE_SGIX (0x8199)";

      /* -------------------------- GL_SGIX_fog_texture
       * -------------------------- */

      /* ------------------- GL_SGIX_fragment_specular_lighting
       * ------------------ */

      /* --------------------------- GL_SGIX_framezoom
       * --------------------------- */

      /* --------------------------- GL_SGIX_interlace
       * --------------------------- */

    case 0x8094 /* GL_INTERLACE_SGIX */:
      return "GL_INTERLACE_SGIX (0x8094)";

      /* ------------------------- GL_SGIX_ir_instrument1
       * ------------------------ */

      /* ------------------------- GL_SGIX_list_priority
       * ------------------------- */

      /* ------------------------- GL_SGIX_pixel_texture
       * ------------------------- */

      /* ----------------------- GL_SGIX_pixel_texture_bits
       * ---------------------- */

      /* ------------------------ GL_SGIX_reference_plane
       * ------------------------ */

      /* ---------------------------- GL_SGIX_resample
       * --------------------------- */

    case 0x842E /* GL_PACK_RESAMPLE_SGIX */:
      return "GL_PACK_RESAMPLE_SGIX (0x842E)";
    case 0x842F /* GL_UNPACK_RESAMPLE_SGIX */:
      return "GL_UNPACK_RESAMPLE_SGIX (0x842F)";
    case 0x8430 /* GL_RESAMPLE_DECIMATE_SGIX */:
      return "GL_RESAMPLE_DECIMATE_SGIX (0x8430)";
    case 0x8433 /* GL_RESAMPLE_REPLICATE_SGIX */:
      return "GL_RESAMPLE_REPLICATE_SGIX (0x8433)";
    case 0x8434 /* GL_RESAMPLE_ZERO_FILL_SGIX */:
      return "GL_RESAMPLE_ZERO_FILL_SGIX (0x8434)";

      /* ----------------------------- GL_SGIX_shadow
       * ---------------------------- */

    case 0x819A /* GL_TEXTURE_COMPARE_SGIX */:
      return "GL_TEXTURE_COMPARE_SGIX (0x819A)";
    case 0x819B /* GL_TEXTURE_COMPARE_OPERATOR_SGIX */:
      return "GL_TEXTURE_COMPARE_OPERATOR_SGIX (0x819B)";
    case 0x819C /* GL_TEXTURE_LEQUAL_R_SGIX */:
      return "GL_TEXTURE_LEQUAL_R_SGIX (0x819C)";
    case 0x819D /* GL_TEXTURE_GEQUAL_R_SGIX */:
      return "GL_TEXTURE_GEQUAL_R_SGIX (0x819D)";

      /* ------------------------- GL_SGIX_shadow_ambient
       * ------------------------ */

      // case 0x80BF /* GL_SHADOW_AMBIENT_SGIX */: return
      // "GL_SHADOW_AMBIENT_SGIX (0x80BF)";

      /* ----------------------------- GL_SGIX_sprite
       * ---------------------------- */

      /* ----------------------- GL_SGIX_tag_sample_buffer
       * ----------------------- */

      /* ------------------------ GL_SGIX_texture_add_env
       * ------------------------ */

      /* -------------------- GL_SGIX_texture_coordinate_clamp
       * ------------------- */

    case 0x8369 /* GL_TEXTURE_MAX_CLAMP_S_SGIX */:
      return "GL_TEXTURE_MAX_CLAMP_S_SGIX (0x8369)";
    case 0x836A /* GL_TEXTURE_MAX_CLAMP_T_SGIX */:
      return "GL_TEXTURE_MAX_CLAMP_T_SGIX (0x836A)";
    case 0x836B /* GL_TEXTURE_MAX_CLAMP_R_SGIX */:
      return "GL_TEXTURE_MAX_CLAMP_R_SGIX (0x836B)";

      /* ------------------------ GL_SGIX_texture_lod_bias
       * ----------------------- */

      /* ---------------------- GL_SGIX_texture_multi_buffer
       * --------------------- */

    case 0x812E /* GL_TEXTURE_MULTI_BUFFER_HINT_SGIX */:
      return "GL_TEXTURE_MULTI_BUFFER_HINT_SGIX (0x812E)";

      /* ------------------------- GL_SGIX_texture_range
       * ------------------------- */

    case 0x85E0 /* GL_RGB_SIGNED_SGIX */:
      return "GL_RGB_SIGNED_SGIX (0x85E0)";
    case 0x85E1 /* GL_RGBA_SIGNED_SGIX */:
      return "GL_RGBA_SIGNED_SGIX (0x85E1)";
    case 0x85E2 /* GL_ALPHA_SIGNED_SGIX */:
      return "GL_ALPHA_SIGNED_SGIX (0x85E2)";
    case 0x85E3 /* GL_LUMINANCE_SIGNED_SGIX */:
      return "GL_LUMINANCE_SIGNED_SGIX (0x85E3)";
    case 0x85E4 /* GL_INTENSITY_SIGNED_SGIX */:
      return "GL_INTENSITY_SIGNED_SGIX (0x85E4)";
    case 0x85E5 /* GL_LUMINANCE_ALPHA_SIGNED_SGIX */:
      return "GL_LUMINANCE_ALPHA_SIGNED_SGIX (0x85E5)";
    case 0x85E6 /* GL_RGB16_SIGNED_SGIX */:
      return "GL_RGB16_SIGNED_SGIX (0x85E6)";
    case 0x85E7 /* GL_RGBA16_SIGNED_SGIX */:
      return "GL_RGBA16_SIGNED_SGIX (0x85E7)";
    case 0x85E8 /* GL_ALPHA16_SIGNED_SGIX */:
      return "GL_ALPHA16_SIGNED_SGIX (0x85E8)";
    case 0x85E9 /* GL_LUMINANCE16_SIGNED_SGIX */:
      return "GL_LUMINANCE16_SIGNED_SGIX (0x85E9)";
    case 0x85EA /* GL_INTENSITY16_SIGNED_SGIX */:
      return "GL_INTENSITY16_SIGNED_SGIX (0x85EA)";
    case 0x85EB /* GL_LUMINANCE16_ALPHA16_SIGNED_SGIX */:
      return "GL_LUMINANCE16_ALPHA16_SIGNED_SGIX (0x85EB)";
    case 0x85EC /* GL_RGB_EXTENDED_RANGE_SGIX */:
      return "GL_RGB_EXTENDED_RANGE_SGIX (0x85EC)";
    case 0x85ED /* GL_RGBA_EXTENDED_RANGE_SGIX */:
      return "GL_RGBA_EXTENDED_RANGE_SGIX (0x85ED)";
    case 0x85EE /* GL_ALPHA_EXTENDED_RANGE_SGIX */:
      return "GL_ALPHA_EXTENDED_RANGE_SGIX (0x85EE)";
    case 0x85EF /* GL_LUMINANCE_EXTENDED_RANGE_SGIX */:
      return "GL_LUMINANCE_EXTENDED_RANGE_SGIX (0x85EF)";
    case 0x85F0 /* GL_INTENSITY_EXTENDED_RANGE_SGIX */:
      return "GL_INTENSITY_EXTENDED_RANGE_SGIX (0x85F0)";
    case 0x85F1 /* GL_LUMINANCE_ALPHA_EXTENDED_RANGE_SGIX */:
      return "GL_LUMINANCE_ALPHA_EXTENDED_RANGE_SGIX (0x85F1)";
    case 0x85F2 /* GL_RGB16_EXTENDED_RANGE_SGIX */:
      return "GL_RGB16_EXTENDED_RANGE_SGIX (0x85F2)";
    case 0x85F3 /* GL_RGBA16_EXTENDED_RANGE_SGIX */:
      return "GL_RGBA16_EXTENDED_RANGE_SGIX (0x85F3)";
    case 0x85F4 /* GL_ALPHA16_EXTENDED_RANGE_SGIX */:
      return "GL_ALPHA16_EXTENDED_RANGE_SGIX (0x85F4)";
    case 0x85F5 /* GL_LUMINANCE16_EXTENDED_RANGE_SGIX */:
      return "GL_LUMINANCE16_EXTENDED_RANGE_SGIX (0x85F5)";
    case 0x85F6 /* GL_INTENSITY16_EXTENDED_RANGE_SGIX */:
      return "GL_INTENSITY16_EXTENDED_RANGE_SGIX (0x85F6)";
    case 0x85F7 /* GL_LUMINANCE16_ALPHA16_EXTENDED_RANGE_SGIX */:
      return "GL_LUMINANCE16_ALPHA16_EXTENDED_RANGE_SGIX (0x85F7)";
    case 0x85F8 /* GL_MIN_LUMINANCE_SGIS */:
      return "GL_MIN_LUMINANCE_SGIS (0x85F8)";
    case 0x85F9 /* GL_MAX_LUMINANCE_SGIS */:
      return "GL_MAX_LUMINANCE_SGIS (0x85F9)";
    case 0x85FA /* GL_MIN_INTENSITY_SGIS */:
      return "GL_MIN_INTENSITY_SGIS (0x85FA)";
    case 0x85FB /* GL_MAX_INTENSITY_SGIS */:
      return "GL_MAX_INTENSITY_SGIS (0x85FB)";

      /* ----------------------- GL_SGIX_texture_scale_bias
       * ---------------------- */

    case 0x8179 /* GL_POST_TEXTURE_FILTER_BIAS_SGIX */:
      return "GL_POST_TEXTURE_FILTER_BIAS_SGIX (0x8179)";
    case 0x817A /* GL_POST_TEXTURE_FILTER_SCALE_SGIX */:
      return "GL_POST_TEXTURE_FILTER_SCALE_SGIX (0x817A)";
    case 0x817B /* GL_POST_TEXTURE_FILTER_BIAS_RANGE_SGIX */:
      return "GL_POST_TEXTURE_FILTER_BIAS_RANGE_SGIX (0x817B)";
    case 0x817C /* GL_POST_TEXTURE_FILTER_SCALE_RANGE_SGIX */:
      return "GL_POST_TEXTURE_FILTER_SCALE_RANGE_SGIX (0x817C)";

      /* ------------------------- GL_SGIX_vertex_preclip
       * ------------------------ */

    case 0x83EE /* GL_VERTEX_PRECLIP_SGIX */:
      return "GL_VERTEX_PRECLIP_SGIX (0x83EE)";
    case 0x83EF /* GL_VERTEX_PRECLIP_HINT_SGIX */:
      return "GL_VERTEX_PRECLIP_HINT_SGIX (0x83EF)";

      /* ---------------------- GL_SGIX_vertex_preclip_hint
       * ---------------------- */

      // case 0x83EE /* GL_VERTEX_PRECLIP_SGIX */: return
      // "GL_VERTEX_PRECLIP_SGIX (0x83EE)"; case 0x83EF /*
      // GL_VERTEX_PRECLIP_HINT_SGIX */: return "GL_VERTEX_PRECLIP_HINT_SGIX
      // (0x83EF)";

      /* ----------------------------- GL_SGIX_ycrcb
       * ----------------------------- */

      /* -------------------------- GL_SGI_color_matrix
       * -------------------------- */

      // case 0x80B1 /* GL_COLOR_MATRIX_SGI */: return "GL_COLOR_MATRIX_SGI
      // (0x80B1)"; case 0x80B2 /* GL_COLOR_MATRIX_STACK_DEPTH_SGI */: return
      // "GL_COLOR_MATRIX_STACK_DEPTH_SGI (0x80B2)"; case 0x80B3 /*
      // GL_MAX_COLOR_MATRIX_STACK_DEPTH_SGI */: return
      // "GL_MAX_COLOR_MATRIX_STACK_DEPTH_SGI (0x80B3)"; case 0x80B4 /*
      // GL_POST_COLOR_MATRIX_RED_SCALE_SGI */: return
      // "GL_POST_COLOR_MATRIX_RED_SCALE_SGI (0x80B4)"; case 0x80B5 /*
      // GL_POST_COLOR_MATRIX_GREEN_SCALE_SGI */: return
      // "GL_POST_COLOR_MATRIX_GREEN_SCALE_SGI (0x80B5)"; case 0x80B6 /*
      // GL_POST_COLOR_MATRIX_BLUE_SCALE_SGI */: return
      // "GL_POST_COLOR_MATRIX_BLUE_SCALE_SGI (0x80B6)"; case 0x80B7 /*
      // GL_POST_COLOR_MATRIX_ALPHA_SCALE_SGI */: return
      // "GL_POST_COLOR_MATRIX_ALPHA_SCALE_SGI (0x80B7)"; case 0x80B8 /*
      // GL_POST_COLOR_MATRIX_RED_BIAS_SGI */: return
      // "GL_POST_COLOR_MATRIX_RED_BIAS_SGI (0x80B8)"; case 0x80B9 /*
      // GL_POST_COLOR_MATRIX_GREEN_BIAS_SGI */: return
      // "GL_POST_COLOR_MATRIX_GREEN_BIAS_SGI (0x80B9)"; case 0x80BA /*
      // GL_POST_COLOR_MATRIX_BLUE_BIAS_SGI */: return
      // "GL_POST_COLOR_MATRIX_BLUE_BIAS_SGI (0x80BA)"; case 0x80BB /*
      // GL_POST_COLOR_MATRIX_ALPHA_BIAS_SGI */: return
      // "GL_POST_COLOR_MATRIX_ALPHA_BIAS_SGI (0x80BB)";

      /* --------------------------- GL_SGI_color_table
       * -------------------------- */

      // case 0x80D0 /* GL_COLOR_TABLE_SGI */: return "GL_COLOR_TABLE_SGI
      // (0x80D0)"; case 0x80D1 /* GL_POST_CONVOLUTION_COLOR_TABLE_SGI */: return
      // "GL_POST_CONVOLUTION_COLOR_TABLE_SGI (0x80D1)"; case 0x80D2 /*
      // GL_POST_COLOR_MATRIX_COLOR_TABLE_SGI */: return
      // "GL_POST_COLOR_MATRIX_COLOR_TABLE_SGI (0x80D2)"; case 0x80D3 /*
      // GL_PROXY_COLOR_TABLE_SGI */: return "GL_PROXY_COLOR_TABLE_SGI (0x80D3)";
      // case 0x80D4 /* GL_PROXY_POST_CONVOLUTION_COLOR_TABLE_SGI */: return
      // "GL_PROXY_POST_CONVOLUTION_COLOR_TABLE_SGI (0x80D4)"; case 0x80D5 /*
      // GL_PROXY_POST_COLOR_MATRIX_COLOR_TABLE_SGI */: return
      // "GL_PROXY_POST_COLOR_MATRIX_COLOR_TABLE_SGI (0x80D5)"; case 0x80D6 /*
      // GL_COLOR_TABLE_SCALE_SGI */: return "GL_COLOR_TABLE_SCALE_SGI (0x80D6)";
      // case 0x80D7 /* GL_COLOR_TABLE_BIAS_SGI */: return
      // "GL_COLOR_TABLE_BIAS_SGI (0x80D7)"; case 0x80D8 /*
      // GL_COLOR_TABLE_FORMAT_SGI */: return "GL_COLOR_TABLE_FORMAT_SGI
      // (0x80D8)"; case 0x80D9 /* GL_COLOR_TABLE_WIDTH_SGI */: return
      // "GL_COLOR_TABLE_WIDTH_SGI (0x80D9)"; case 0x80DA /*
      // GL_COLOR_TABLE_RED_SIZE_SGI */: return "GL_COLOR_TABLE_RED_SIZE_SGI
      // (0x80DA)"; case 0x80DB /* GL_COLOR_TABLE_GREEN_SIZE_SGI */: return
      // "GL_COLOR_TABLE_GREEN_SIZE_SGI (0x80DB)"; case 0x80DC /*
      // GL_COLOR_TABLE_BLUE_SIZE_SGI */: return "GL_COLOR_TABLE_BLUE_SIZE_SGI
      // (0x80DC)"; case 0x80DD /* GL_COLOR_TABLE_ALPHA_SIZE_SGI */: return
      // "GL_COLOR_TABLE_ALPHA_SIZE_SGI (0x80DD)"; case 0x80DE /*
      // GL_COLOR_TABLE_LUMINANCE_SIZE_SGI */: return
      // "GL_COLOR_TABLE_LUMINANCE_SIZE_SGI (0x80DE)"; case 0x80DF /*
      // GL_COLOR_TABLE_INTENSITY_SIZE_SGI */: return
      // "GL_COLOR_TABLE_INTENSITY_SIZE_SGI (0x80DF)";

      /* ----------------------- GL_SGI_texture_color_table
       * ---------------------- */

    case 0x80BC /* GL_TEXTURE_COLOR_TABLE_SGI */:
      return "GL_TEXTURE_COLOR_TABLE_SGI (0x80BC)";
    case 0x80BD /* GL_PROXY_TEXTURE_COLOR_TABLE_SGI */:
      return "GL_PROXY_TEXTURE_COLOR_TABLE_SGI (0x80BD)";

      /* ------------------------- GL_SUNX_constant_data
       * ------------------------- */

    case 0x81D5 /* GL_UNPACK_CONSTANT_DATA_SUNX */:
      return "GL_UNPACK_CONSTANT_DATA_SUNX (0x81D5)";
    case 0x81D6 /* GL_TEXTURE_CONSTANT_DATA_SUNX */:
      return "GL_TEXTURE_CONSTANT_DATA_SUNX (0x81D6)"; /* --------------------
                                                          GL_SUN_convolution_border_modes
                                                          --------------------
                                                        */

    case 0x81D4 /* GL_WRAP_BORDER_SUN */:
      return "GL_WRAP_BORDER_SUN (0x81D4)";

      /* -------------------------- GL_SUN_global_alpha
       * -------------------------- */

    case 0x81D9 /* GL_GLOBAL_ALPHA_SUN */:
      return "GL_GLOBAL_ALPHA_SUN (0x81D9)";
    case 0x81DA /* GL_GLOBAL_ALPHA_FACTOR_SUN */:
      return "GL_GLOBAL_ALPHA_FACTOR_SUN (0x81DA)";

      /* --------------------------- GL_SUN_mesh_array
       * --------------------------- */

    case 0x8614 /* GL_QUAD_MESH_SUN */:
      return "GL_QUAD_MESH_SUN (0x8614)";
    case 0x8615 /* GL_TRIANGLE_MESH_SUN */:
      return "GL_TRIANGLE_MESH_SUN (0x8615)";

      /* ------------------------ GL_SUN_read_video_pixels
       * ----------------------- */

      /* --------------------------- GL_SUN_slice_accum
       * -------------------------- */

    case 0x85CC /* GL_SLICE_ACCUM_SUN */:
      return "GL_SLICE_ACCUM_SUN (0x85CC)";

    /* -------------------------- GL_SUN_triangle_list -------------------------
     */

    // case 0x01 /* GL_RESTART_SUN */: return "GL_RESTART_SUN (0x01)";
    // case 0x02 /* GL_REPLACE_MIDDLE_SUN */: return "GL_REPLACE_MIDDLE_SUN
    // (0x02)"; case 0x03 /* GL_REPLACE_OLDEST_SUN */: return
    // "GL_REPLACE_OLDEST_SUN (0x03)";
    case 0x81D7 /* GL_TRIANGLE_LIST_SUN */:
      return "GL_TRIANGLE_LIST_SUN (0x81D7)";
    case 0x81D8 /* GL_REPLACEMENT_CODE_SUN */:
      return "GL_REPLACEMENT_CODE_SUN (0x81D8)";
    case 0x85C0 /* GL_REPLACEMENT_CODE_ARRAY_SUN */:
      return "GL_REPLACEMENT_CODE_ARRAY_SUN (0x85C0)";
    case 0x85C1 /* GL_REPLACEMENT_CODE_ARRAY_TYPE_SUN */:
      return "GL_REPLACEMENT_CODE_ARRAY_TYPE_SUN (0x85C1)";
    case 0x85C2 /* GL_REPLACEMENT_CODE_ARRAY_STRIDE_SUN */:
      return "GL_REPLACEMENT_CODE_ARRAY_STRIDE_SUN (0x85C2)";
    case 0x85C3 /* GL_REPLACEMENT_CODE_ARRAY_POINTER_SUN */:
      return "GL_REPLACEMENT_CODE_ARRAY_POINTER_SUN (0x85C3)";
    case 0x85C4 /* GL_R1UI_V3F_SUN */:
      return "GL_R1UI_V3F_SUN (0x85C4)";
    case 0x85C5 /* GL_R1UI_C4UB_V3F_SUN */:
      return "GL_R1UI_C4UB_V3F_SUN (0x85C5)";
    case 0x85C6 /* GL_R1UI_C3F_V3F_SUN */:
      return "GL_R1UI_C3F_V3F_SUN (0x85C6)";
    case 0x85C7 /* GL_R1UI_N3F_V3F_SUN */:
      return "GL_R1UI_N3F_V3F_SUN (0x85C7)";
    case 0x85C8 /* GL_R1UI_C4F_N3F_V3F_SUN */:
      return "GL_R1UI_C4F_N3F_V3F_SUN (0x85C8)";
    case 0x85C9 /* GL_R1UI_T2F_V3F_SUN */:
      return "GL_R1UI_T2F_V3F_SUN (0x85C9)";
    case 0x85CA /* GL_R1UI_T2F_N3F_V3F_SUN */:
      return "GL_R1UI_T2F_N3F_V3F_SUN (0x85CA)";
    case 0x85CB /* GL_R1UI_T2F_C4F_N3F_V3F_SUN */:
      return "GL_R1UI_T2F_C4F_N3F_V3F_SUN (0x85CB)";

      /* ----------------------------- GL_SUN_vertex
       * ----------------------------- */
      /* -------------------------- GL_WIN_phong_shading
       * ------------------------- */

    case 0x80EA /* GL_PHONG_WIN */:
      return "GL_PHONG_WIN (0x80EA)";
    case 0x80EB /* GL_PHONG_HINT_WIN */:
      return "GL_PHONG_HINT_WIN (0x80EB)";

      /* -------------------------- GL_WIN_specular_fog
       * -------------------------- */

    case 0x80EC /* GL_FOG_SPECULAR_TEXTURE_WIN */:
      return "GL_FOG_SPECULAR_TEXTURE_WIN (0x80EC)";

    default: {
      return "what?";
    }
  } // ~switch

} // ~WWWGLEnumToString

int main() {
  volatile int i;
  i = 34962;
  printf("%d: %s\n", i, WWWGLEnumToString(i));
  i = 26214;
  printf("%d: %s\n", i, WWWGLEnumToString(i));
  i = 35040;
  printf("%d: %s\n", i, WWWGLEnumToString(i));
  i = 3060;
  printf("%d: %s\n", i, WWWGLEnumToString(i));
}
