/**
*****************************************************************************************
*     Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
* @file    ftl_map.h
* @brief   FTL Layout Configuration!
* @note    The ftl APIs ftl_save/ftl_load are used by multiple modules in the mesh stack and the application.
*          The application shall plan the ftl usage as a whole in case of the overlap of the ftl address.
* @author  bill
* @date    2019-04-28
* @version v1.0
* *************************************************************************************
*/

#ifndef _FTL_MAP_H_
#define _FTL_MAP_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
 * The ftl physical size is FTL_SIZE defined in flash_map.h.
 * The ftl logical size is calculated approximately by (FTL_SIZE - 4K)/2 bytes.
 * Example: if FTL_SIZE=32KB, ftl logic size is approximately equal to 14KB.
 *
 * The ftl space is shared by the bt stack, the mesh stack and the application.
 * The bt stack takes about 3KB space of the front part the ftl logical space defautly.
 * Nevertheless, the APIs ftl_save/ftl_load have subtracted the ftl logical space used the bt stack.
 * That is to say, the input parameter offset of APIs ftl_save/ftl_load is zero-based,
 * the size parameter shall be eqaul or less than the ftl logical size - 3KB.
 */

/* mesh stack */
#define FTL_MAP_MESH_STACK_OFFSET       0 //!< it can be used to modify the mesh_node.flash_offset (default 0)
#define FTL_MAP_MESH_STACK_SIZE         1800 //!< it can be used to modify the mesh_node.flash_size (defualt 1800)

/* dimmable light */
#define FTL_MAP_DIMMABLE_LIGHT_OFFSET   1900
#define FTL_MAP_DIMMABLE_LIGHT_SIZE     100

/* mesh dfu updater */
#define FTL_MAP_MESH_DFU_UPDATER_OFFSET 2000
#define FTL_MAP_MESH_DFU_UPDATER_SIZE   sizeof(dfu_updater_nvm_info_t)

/*  */

#ifdef __cplusplus
}
#endif
/** @} */ /* _FTL_MAP_H_ */
#endif
