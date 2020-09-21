#include "rig.h"
#include "model.h"
#include "sdl.h"
#include "mat.h"

extern struct model_rig *rig_derive(short slot)
{
	struct model_rig *rig;
	struct model *mdl;
	int tmp;
	int i;

	if(mdl_check_slot(slot))
		return NULL;

	mdl = models[slot];
	if(!mdl || mdl->status != MDL_OK)
		return NULL;

	if(mdl->type < MDL_RIG)
		return NULL;

	if(!(rig = malloc(sizeof(struct model_rig))))
		return NULL;

	rig->model = slot;
	rig->anim = 0;
	rig->prog = 0;
	rig->ts = 0;

	rig->jnt_num = mdl->jnt_num;

	tmp = rig->jnt_num * 16 * sizeof(float);
	if(!(rig->jnt_mat = malloc(tmp)))
		goto err_free_rig;

	/* Initialize joint-matrices as identitiy-matrices */
	for(i = 0; i < rig->jnt_num; i++)
		mat4_idt(rig->jnt_mat + (i * 16));

	return rig;

err_free_rig:
	free(rig);
	return NULL;
}


extern void rig_free(struct model_rig *rig)
{
	if(!rig)
		return;

	free(rig->jnt_mat);
	free(rig);
}


extern void rig_update(void)
{

}
