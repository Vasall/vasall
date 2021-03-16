/*
 * zlib License
 * 
 * (C) 2021 clusterwerk
 * 
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 * 
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 * 
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 * 
 */

#ifndef _AMOLOADER_H
#define _AMOLOADER_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define AMO_API extern
#define AMO_INTERN static

/*
 * The different data-formats specifying which data is contained in the returned
 * data-struct.
 */
enum amo_format {
	AMO_FORMAT_NONE,
	AMO_FORMAT_OBJ,
	AMO_FORMAT_AMO
};


/*
 * A struct representing a simple collision box.
 *
 * @position: The position of the collision-box relative to the origion
 * @size: The size of the collision-box
 */
struct amo_shape3d {
	float pos[3];
	float scl[3];
};


/*
 * A struct symbolising a joint of the model.
 * 
 * @name: The name of the joint
 * @index: The index of the joint in the joint-array
 * @par: A pointer to the parent joint
 */
struct amo_joint;
struct amo_joint {
	char              name[100];
	int               index;
	struct amo_joint  *par;
	float             mat[16];
};

/*
 * A struct representing a single keyframe of an animation.
 *
 * @prog: The progression at which the keyframe occurrs
 * @joints: An array containing pointers to the according joints
 * @pos: The position of a joint in this keyframe
 * @rot: The rotation of a joint in this keyframe
 */
struct amo_keyfr {
	float prog;

	int jnt_num;

	struct amo_joint **joints;

	short             *jnt;
	float             *pos;
	float             *rot;
};

/*
 * A struct containing information about one animation cycle
 * 
 * @name: The name of the animation
 * @dur: The duration of the animation in milliseconds
 * @keyfr_c: The number of keyframes
 * @keyfr_lst: An array containing all keyframes of the animation
 */
struct amo_anim {
	char                  name[100];

	int                   dur;

	int                   keyfr_c;
	struct amo_keyfr      *keyfr_lst;
};

/*
 * The mask for the different data-attributes.
 */
#define AMO_M_NONE 0
#define AMO_M_MDL (1<<0)
#define AMO_M_RIG (1<<1)
#define AMO_M_ANI (1<<2)

#define AMO_M_CBP (1<<3)
#define AMO_M_CNE (1<<4)
#define AMO_M_CCM (1<<5)


/*
 * A struct containing the parsed mdl of the amo file
 * 
 * @name: The name of the object
 * @vtx_c: The amount of vertices
 * @vtx_buf: A 3-dimensional array of vertex positions
 * @tex_buf: A 2-dimensional array of uv coordinates
 * @nrm_buf: A 3-dimensional array of vertex normals
 * @vjnt_buf: A 4-dimensional array of joints affecting the vertices
 * @wgt_buf: A 4-dimensional array of weights affecting the vertices
 * @idx_c: The amount of indices
 * @idx_buf: An array of indices
 * @jnt_c: The amount of joints
 * @jnt_lst: An array of the joints of the model
 * @ani_c: The amount of animations
 * @ani_lst: An array of animations of the model
 */
struct amo_model {
	char                name[100];
	enum amo_format     format;
	unsigned int        attr_m;

	int                 vtx_c;
	float               *vtx_buf;

	int                 tex_c;
	float               *tex_buf;

	int                 nrm_c;
	float               *nrm_buf;

	int                 vjnt_c;
	int                 *vjnt_buf;

	int                 wgt_c;
	float               *wgt_buf;

	int                 idx_c;
	unsigned int        *idx_buf;

	/*
	 * joints
	 */

	int                 jnt_c;
	struct amo_joint    *jnt_lst;

	/*
	 * animations
	 */

	int                 ani_c;
	struct amo_anim     *ani_lst;

	/*
	 * collision-buffers 
	 */

	struct amo_shape3d  bp_col;
	struct amo_shape3d  ne_col;

	int                 cm_vtx_c;
	float               *cm_vtx_buf;
	int                 cm_idx_c;
	int                 *cm_idx_buf;
	float               *cm_nrm_buf;
};

/*
 * Loads the dot-amo file and parses its values into amo_model.
 * 
 * @pth: The file path to the dot-amo file
 * 
 * Returns: Returns a buffer containing the model-data read from the given
 *          dot-amo file
 */
AMO_API struct amo_model *amo_load(const char *pth);


/*
 * Destroys the amo_model struct.
 * 
 * @data: An array of models previously created by amo_load()
 * 
 * Returns: 0 for success or -1 if an error occurred
 */
AMO_API void amo_destroy(struct amo_model *data);


/*
 * Get the mesh and vertex-joints of the model in an OpenGL-compatible-format
 * where the data for each vertex is stored in the corresponding array and the#
 * index-array contains the vertex-indices.
 * Note that the returned arrays have to be freed after usage, to prevent
 * memory-leaks.
 *
 * @data: Pointer to the data-buffer
 * @vtxnum: A pointer to write the vertex-number to
 * @vtx: A pointer to attach the vertex-buffer to
 * @tex: A pointer to attach the texture-coordinate-buffer to
 * @nrm: A pointer to attach the normal-buffer to
 * @jnt: A pointer to attach the joint-index-buffer to
 * @wgt: A pointer to attach the joint-weight-buffer to
 * @idxnum: A pointer to write the index-number to
 * @idx: A pointer to attach the index-buffer to
 *
 * Returns: 0 on success or -1 if an error occurred
 */
AMO_API int amo_getdata(struct amo_model *data, int *vtxnum, void **vtx,
		void **tex, void **nrm, void **jnt, void **wgt, int *idxnum,
		unsigned int **idx);

#endif
