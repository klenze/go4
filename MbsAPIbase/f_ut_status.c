// $Id$
//-----------------------------------------------------------------------
//       The GSI Online Offline Object Oriented (Go4) Project
//         Experiment Data Processing at EE department, GSI
//-----------------------------------------------------------------------
// Copyright (C) 2000- GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
//                     Planckstr. 1, 64291 Darmstadt, Germany
// Contact:            http://go4.gsi.de
//-----------------------------------------------------------------------
// This software can be used under the license agreements as stated
// in Go4License.txt file which is part of the distribution.
//-----------------------------------------------------------------------
//
// NOTE !!!!!!!
// To avoid hanging sockets on Lynx, we read one byte less than MBS sends.
// This byte is the last of process name table, so never used.
// In MBS the problem can be fixed. Then this workaround can be rolled back.
// Reason is that client must close socket before server.

#include "typedefs.h"
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <memory.h>
#include <sys/types.h>
#include <ctype.h>

#include "s_daqst.h"
#include "s_setup.h"
#include "s_set_ml.h"
#include "s_set_mo.h"
#include "f_stccomm.h"
#include "f_swaplw.h"

#define VERSION__DAQST  63
#define VERSION__SETUP  63
#define VERSION__SET_ML 63
#define VERSION__SET_MO 63
/*****************+***********+****************************************/
/*                                                                    */
/*   GSI, Gesellschaft fuer Schwerionenforschung mbH                  */
/*   Postfach 11 05 52                                                */
/*   D-64220 Darmstadt                                                */
/*                                                                    */
/*1+ C Procedure *************+****************************************/
/*                                                                    */
/*+ Module      : f_ut_status_ini                                     */
/*                                                                    */
/*--------------------------------------------------------------------*/
/*+ CALLING     : l_status=f_ut_status_ini(s_daqst)                   */
/*--------------------------------------------------------------------*/
/*                                                                    */
/*+ PURPOSE     : Initialize s_daqst.                                 */
/*                                                                    */
/*+ ARGUMENTS   :                                                     */
/*+  s_daqst    : pointer to s_daqst                                  */
/*2+Implementation************+****************************************/
/*+ File name   : f_ut_status.c                                       */
/*+ Home direct.: /mbs/prod/src                                       */
/*+ Version     : 2.01                                                */
/*+ Author      : H.G.Essel                                           */
/*+ Created     : 14-Nov-1998                                         */
/*+ Object libr.: mbslib.a                                            */
/*+ Updates     : Date        Purpose                                 */
/*1- C Procedure *************+****************************************/
INTS4 f_ut_status_ini(s_daqst *ps_daqst)
{
 if(ps_daqst == NULL) return -1;

 ps_daqst->l_endian            = 1;
 ps_daqst->l_version           = VERSION__DAQST;
 ps_daqst->l_daqst_lw          = sizeof(s_daqst)/4;
 ps_daqst->l_fix_lw            = ((ADDRS)&ps_daqst->c_pname[0][0]-(ADDRS)ps_daqst)/4;
 ps_daqst->l_sys__n_max_procs  = SYS__N_MAX_PROCS;
 ps_daqst->l_sbs__str_len_64   = SBS__STR_LEN_64;
 ps_daqst->l_sbs__n_trg_typ    = SBS__N_TRG_TYP;
 ps_daqst->bh_daqst_initalized = 1;

 return 0;
}


/*****************+***********+****************************************/
/*                                                                    */
/*   GSI, Gesellschaft fuer Schwerionenforschung mbH                  */
/*   Postfach 11 05 52                                                */
/*   D-64220 Darmstadt                                                */
/*                                                                    */
/*1+ C Procedure *************+****************************************/
/*                                                                    */
/*+ Module      : f_ut_status                                         */
/*                                                                    */
/*--------------------------------------------------------------------*/
/*+ CALLING     : l_status=f_ut_status(s_daqst, socket)               */
/*--------------------------------------------------------------------*/
/*                                                                    */
/*+ PURPOSE     : Send s_daqst to socket.                             */
/*                                                                    */
/*+ ARGUMENTS   :                                                     */
/*+  s_daqst    : pointer to s_daqst                                  */
/*+  socket     : Tcp socket from accept                              */
/*2+Implementation************+****************************************/
/*+ File name   : f_ut_status.c                                       */
/*+ Home direct.: /mbs/prod/src                                       */
/*+ Version     : 2.01                                                */
/*+ Author      : H.G.Essel                                           */
/*+ Created     : 14-Nov-1998                                         */
/*+ Object libr.: mbslib.a                                            */
/*+ Updates     : Date        Purpose                                 */
/*1- C Procedure *************+****************************************/
INTS4 f_ut_status(s_daqst *ps_daqst, INTS4 l_tcp)
{
INTS4 l,l_status;

l = ps_daqst->l_fix_lw * 4 + ps_daqst->l_procs_run * ps_daqst->l_sbs__str_len_64;
/*printf("daqst Send %d LW + %d LW",ps_daqst->l_fix_lw,ps_daqst->l_procs_run * ps_daqst->l_sbs__str_len_64/4);*/
l_status = f_stc_write (ps_daqst, l, l_tcp);
return l_status;
}

/*****************+***********+****************************************/
/*                                                                    */
/*   GSI, Gesellschaft fuer Schwerionenforschung mbH                  */
/*   Postfach 11 05 52                                                */
/*   D-64220 Darmstadt                                                */
/*                                                                    */
/*1+ C Procedure *************+****************************************/
/*                                                                    */
/*+ Module      : f_ut_status_r                                       */
/*                                                                    */
/*--------------------------------------------------------------------*/
/*+ CALLING     : l_status=f_ut_status_r(s_daqst, socket)             */
/*--------------------------------------------------------------------*/
/*                                                                    */
/*+ PURPOSE     : Read s_daqst from socket.                           */
/*                                                                    */
/*+ ARGUMENTS   :                                                     */
/*+  s_daqst    : pointer to s_daqst                                  */
/*+  socket     : Tcp socket from connect                             */
/*2+Implementation************+****************************************/
/*+ File name   : f_ut_status.c                                       */
/*+ Home direct.: /mbs/prod/src                                       */
/*+ Version     : 2.01                                                */
/*+ Author      : H.G.Essel                                           */
/*+ Created     : 14-Nov-1998                                         */
/*+ Object libr.: mbslib.a                                            */
/*+ Updates     : Date        Purpose                                 */
/*1- C Procedure *************+****************************************/
INTS4 f_ut_status_r(s_daqst *ps_daqst, INTS4 l_tcp)
{
  INTS4 l_swap=0, len_64, n_trg, max_proc;
  INTS4 l_cmd;
  INTS4 k;
  INTS4 l_status;

  memset((void *)ps_daqst,0,sizeof(s_daqst));
  l_cmd=1;
  l_status = f_stc_write (&l_cmd,4,l_tcp);                 if (l_status != STC__SUCCESS) return(-1);
  l_status = f_stc_read (&ps_daqst->l_endian,28,l_tcp,-1); if (l_status != STC__SUCCESS) return(-1);
  if(ps_daqst->l_endian != 1) l_swap = 1;
  if(l_swap == 1) l_status = f_swaplw((INTS4 *)&ps_daqst->l_endian,7,NULL);
  len_64=ps_daqst->l_sbs__str_len_64;
  n_trg=ps_daqst->l_sbs__n_trg_typ;
  max_proc=ps_daqst->l_sys__n_max_procs;
  if(ps_daqst->l_version == 1)
  {
	  // MBS v44 and previous no longer supported
	  return(-1);
  }
  // MBS v50
  if(ps_daqst->l_version == 2)
    {
	  k=(48+n_trg*3)*4; // up to bl_n_evt inclusive
      l_status = f_stc_read (&ps_daqst->bh_daqst_initalized, k , l_tcp,-1);
	  k=(24+max_proc*5)*4; // bh_running up to bl_event_build_on inclusive
      l_status = f_stc_read (&ps_daqst->bh_running[0], k , l_tcp,-1);
	  k=len_64*15; // strings up to c_file_name inclusive
      l_status = f_stc_read (&ps_daqst->c_user[0], k , l_tcp,-1);
      l_status = f_stc_read (&ps_daqst->c_out_chan[0], len_64 , l_tcp,-1);
      ps_daqst->l_fix_lw += n_trg*3 + 212 + len_64/4*3;
      if(l_swap == 1)
    	  l_status = f_swaplw((INTS4 *)&ps_daqst->bh_daqst_initalized, (ps_daqst->l_fix_lw-7) - (19 * len_64/4),NULL);
    }
  // MBS v51 and v62/v63
  if((ps_daqst->l_version == 51) || (ps_daqst->l_version == 62) || (ps_daqst->l_version == 63))
    {
      l_status = f_stc_read (&ps_daqst->bh_daqst_initalized, (ps_daqst->l_fix_lw-7)*4 , l_tcp,-1);
      if(l_swap == 1)
    	  l_status = f_swaplw((INTS4 *)&ps_daqst->bh_daqst_initalized, (ps_daqst->l_fix_lw-7) - (19 * len_64/4),NULL);
    }

  //l_status = f_stc_read (&ps_daqst->c_pname[0], ps_daqst->l_procs_run * len_64, l_tcp,-1);
  // workaround:
  l_status = f_stc_read (&ps_daqst->c_pname[0], ps_daqst->l_procs_run * len_64 -1, l_tcp,-1);
  return l_status;
}

/*****************+***********+****************************************/
/*                                                                    */
/*   GSI, Gesellschaft fuer Schwerionenforschung mbH                  */
/*   Postfach 11 05 52                                                */
/*   D-64220 Darmstadt                                                */
/*                                                                    */
/*1+ C Procedure *************+****************************************/
/*                                                                    */
/*+ Module      : f_ut_setup_ini                                      */
/*                                                                    */
/*--------------------------------------------------------------------*/
/*+ CALLING     : l_status=f_ut_setup_ini(s_setup)                    */
/*--------------------------------------------------------------------*/
/*                                                                    */
/*+ PURPOSE     : Initialize s_setup.                                 */
/*                                                                    */
/*+ ARGUMENTS   :                                                     */
/*+  s_setup    : pointer to s_setup                                  */
/*2+Implementation************+****************************************/
/*+ File name   : f_ut_status.c                                       */
/*+ Home direct.: /mbs/prod/src                                       */
/*+ Version     : 2.01                                                */
/*+ Author      : H.G.Essel                                           */
/*+ Created     : 14-Nov-1998                                         */
/*+ Object libr.: mbslib.a                                            */
/*+ Updates     : Date        Purpose                                 */
/*1- C Procedure *************+****************************************/
INTS4 f_ut_setup_ini(s_setup *ps_setup)
{
if(ps_setup == NULL) return -1;
ps_setup->l_endian     = 1;
ps_setup->l_version    = VERSION__SETUP;
ps_setup->bl_struc_len = sizeof(s_setup)/4;
ps_setup->l_fix_lw     = ((ADDRS)&ps_setup->lp_rem_mem_base[0]-(ADDRS)ps_setup)/4;
ps_setup->bl_sbs__n_cr = SBS__N_CR;
ps_setup->bl_sbs__n_trg_typ = SBS__N_TRG_TYP;
return 0;
}



/*****************+***********+****************************************/
/*                                                                    */
/*   GSI, Gesellschaft fuer Schwerionenforschung mbH                  */
/*   Postfach 11 05 52                                                */
/*   D-64220 Darmstadt                                                */
/*                                                                    */
/*1+ C Procedure *************+****************************************/
/*                                                                    */
/*+ Module      : f_ut_setup_copy64                                  */
/*                                                                    */
/*--------------------------------------------------------------------*/
/*+ CALLING     : l_status=f_ut_setup_copy64(s_setup, s_setup_64_receiver)                   */
/*--------------------------------------------------------------------*/
/*                                                                    */
/*+ PURPOSE     : copy 64 bit setup structure to local setup          */
/*                                                                    */
/*+ ARGUMENTS   :                                                     */
/*+  s_setup    : pointer to s_setup
 *   s_setup_64_receiver : pointer to helper structure
 *                                  */
/*2+Implementation************+****************************************/
/*+ File name   : f_ut_setup_copy64                                   */
/*+ Home direct.: /mbs/prod/src                                       */
/*+ Version     : 1.00                                                */
/*+ Author      : J.Adamczewski-Musch                                 */
/*+ Created     : 6-Feb-2015                                         */
/*+ Object libr.: mbslib.a                                            */
/*+ Updates     : Date        Purpose                                 */
/*1- C Procedure *************+****************************************/
INTS4 f_ut_setup_copy64(s_setup *ps_setup, s_setup_64_receiver* src)
{
 if(ps_setup == NULL || src == NULL) return -1;
 ps_setup->bl_sbs__n_cr = src->bl_sbs__n_cr;
 ps_setup->bl_sbs__n_trg_typ = src->bl_sbs__n_trg_typ;
 ps_setup->bi_master=src->bi_master;
 ps_setup->bl_no_crates=src->bl_no_crates;
 ps_setup->bh_crate_nr=src->bh_crate_nr;
 ps_setup->bl_ev_buf_len=src->bl_ev_buf_len;
 ps_setup->bl_n_ev_buf=src->bl_n_ev_buf;
 ps_setup->bl_n_stream=src->bl_n_stream;
 ps_setup->bi_evt_typ_sy= src->bi_evt_typ_sy;
 ps_setup->bi_evt_typ_asy=src->bi_evt_typ_asy;
 ps_setup->bi_evt_subtyp_sy=src->bi_evt_subtyp_sy;
 ps_setup->bi_evt_subtyp_asy=src->bi_evt_subtyp_asy;
 ps_setup->h_se_control=src->h_se_control;
 ps_setup->bh_rd_typ=src->bh_rd_typ;
 ps_setup->bh_col_mode=src->bh_col_mode;
 ps_setup->bl_loc_esone_base=src->bl_loc_esone_base;
 ps_setup->bl_rem_esone_base=src->bl_rem_esone_base;
 ps_setup->bl_esone_off=src->bl_esone_off;
 ps_setup->bl_cvc_crr_off=src->bl_cvc_crr_off;
 ps_setup->bl_cvc_csr_off=src->bl_cvc_csr_off;
 ps_setup->bl_cvc_clb_off=src->bl_cvc_clb_off;
 ps_setup->bl_se_meb_asy_len=src->bl_se_meb_asy_len;
 ps_setup->bh_meb_asy_flg=src->bh_meb_asy_flg;
 ps_setup->bl_ml_pipe_base_addr=src->bl_ml_pipe_base_addr;
 ps_setup->bl_ml_pipe_seg_len=src->bl_ml_pipe_seg_len;
 ps_setup->bl_ml_pipe_len=src->bl_ml_pipe_len;
 ps_setup->bl_n_col_retry=src->bl_n_col_retry;
 ps_setup->bh_meb_trig_mode=src->bh_meb_trig_mode;
 ps_setup->bh_special_meb_trig_type=src->bh_special_meb_trig_type;
 ps_setup->bl_special_meb_trig_base=src->bl_special_meb_trig_base;
 ps_setup->lp_cvc_irq=src->lp_cvc_irq;
 ps_setup->bl_cvc_irq_len=src->bl_cvc_irq_len;
 ps_setup->bl_cvc_irq_source_off=src->bl_cvc_irq_source_off;
 ps_setup->bl_cvc_irq_mask_off=src->bl_cvc_irq_mask_off;
 ps_setup->h_rd_tab_flg=src->h_rd_tab_flg;
 ps_setup->bl_init_read_len=src->bl_init_read_len;
 return 0;
}



/*****************+***********+****************************************/
/*                                                                    */
/*   GSI, Gesellschaft fuer Schwerionenforschung mbH                  */
/*   Postfach 11 05 52                                                */
/*   D-64220 Darmstadt                                                */
/*                                                                    */
/*1+ C Procedure *************+****************************************/
/*                                                                    */
/*+ Module      : f_ut_setup_copy32                                  */
/*                                                                    */
/*--------------------------------------------------------------------*/
/*+ CALLING     : l_status=f_ut_setup_copy32(s_setup, s_setup_32_receiver)                   */
/*--------------------------------------------------------------------*/
/*                                                                    */
/*+ PURPOSE     : copy 32 bit setup structure to local setup          */
/*                                                                    */
/*+ ARGUMENTS   :                                                     */
/*+  s_setup    : pointer to s_setup
 *   s_setup_64_receiver : pointer to helper structure
 *                                  */
/*2+Implementation************+****************************************/
/*+ File name   : f_ut_setup_copy64                                   */
/*+ Home direct.: /mbs/prod/src                                       */
/*+ Version     : 1.00                                                */
/*+ Author      : J.Adamczewski-Musch                                 */
/*+ Created     : 6-Feb-2015                                         */
/*+ Object libr.: mbslib.a                                            */
/*+ Updates     : Date        Purpose                                 */
/*1- C Procedure *************+****************************************/
INTS4 f_ut_setup_copy32(s_setup *ps_setup, s_setup_32_receiver* src)
{
 if(ps_setup == NULL || src == NULL) return -1;
 ps_setup->bl_sbs__n_cr = src->bl_sbs__n_cr;
 ps_setup->bl_sbs__n_trg_typ = src->bl_sbs__n_trg_typ;
 ps_setup->bi_master=src->bi_master;
 ps_setup->bl_no_crates=src->bl_no_crates;
 ps_setup->bh_crate_nr=src->bh_crate_nr;
 ps_setup->bl_ev_buf_len=src->bl_ev_buf_len;
 ps_setup->bl_n_ev_buf=src->bl_n_ev_buf;
 ps_setup->bl_n_stream=src->bl_n_stream;
 ps_setup->bi_evt_typ_sy= src->bi_evt_typ_sy;
 ps_setup->bi_evt_typ_asy=src->bi_evt_typ_asy;
 ps_setup->bi_evt_subtyp_sy=src->bi_evt_subtyp_sy;
 ps_setup->bi_evt_subtyp_asy=src->bi_evt_subtyp_asy;
 ps_setup->h_se_control=src->h_se_control;
 ps_setup->bh_rd_typ=src->bh_rd_typ;
 ps_setup->bh_col_mode=src->bh_col_mode;
 ps_setup->bl_loc_esone_base=src->bl_loc_esone_base;
 ps_setup->bl_rem_esone_base=src->bl_rem_esone_base;
 ps_setup->bl_esone_off=src->bl_esone_off;
 ps_setup->bl_cvc_crr_off=src->bl_cvc_crr_off;
 ps_setup->bl_cvc_csr_off=src->bl_cvc_csr_off;
 ps_setup->bl_cvc_clb_off=src->bl_cvc_clb_off;
 ps_setup->bl_se_meb_asy_len=src->bl_se_meb_asy_len;
 ps_setup->bh_meb_asy_flg=src->bh_meb_asy_flg;
 ps_setup->bl_ml_pipe_base_addr=src->bl_ml_pipe_base_addr;
 ps_setup->bl_ml_pipe_seg_len=src->bl_ml_pipe_seg_len;
 ps_setup->bl_ml_pipe_len=src->bl_ml_pipe_len;
 ps_setup->bl_n_col_retry=src->bl_n_col_retry;
 ps_setup->bh_meb_trig_mode=src->bh_meb_trig_mode;
 ps_setup->bh_special_meb_trig_type=src->bh_special_meb_trig_type;
 ps_setup->bl_special_meb_trig_base=src->bl_special_meb_trig_base;
 ps_setup->lp_cvc_irq=src->lp_cvc_irq;
 ps_setup->bl_cvc_irq_len=src->bl_cvc_irq_len;
 ps_setup->bl_cvc_irq_source_off=src->bl_cvc_irq_source_off;
 ps_setup->bl_cvc_irq_mask_off=src->bl_cvc_irq_mask_off;
 ps_setup->h_rd_tab_flg=src->h_rd_tab_flg;
 ps_setup->bl_init_read_len=src->bl_init_read_len;
 return 0;
}




/*****************+***********+****************************************/
/*                                                                    */
/*   GSI, Gesellschaft fuer Schwerionenforschung mbH                  */
/*   Postfach 11 05 52                                                */
/*   D-64220 Darmstadt                                                */
/*                                                                    */
/*1+ C Procedure *************+****************************************/
/*                                                                    */
/*+ Module      : f_ut_setup                                          */
/*                                                                    */
/*--------------------------------------------------------------------*/
/*+ CALLING     : l_status=f_ut_setup(s_setup, s_setup, socket)       */
/*--------------------------------------------------------------------*/
/*                                                                    */
/*+ PURPOSE     : Write s_setup to socket.                            */
/*                                                                    */
/*+ ARGUMENTS   :                                                     */
/*+  s_setup    : pointer to s_setup                                  */
/*+  s_setup    : pointer to copy of s_setup                          */
/*+  socket     : Tcp socket from accept                              */
/*2+Implementation************+****************************************/
/*+ File name   : f_ut_status.c                                       */
/*+ Home direct.: /mbs/prod/src                                       */
/*+ Version     : 2.01                                                */
/*+ Author      : H.G.Essel                                           */
/*+ Created     : 14-Nov-1998                                         */
/*+ Object libr.: mbslib.a                                            */
/*+ Updates     : Date        Purpose                                 */
/*1- C Procedure *************+****************************************/
INTS4 f_ut_setup(s_setup *ps_setup, INTU4 *pl_o, INTS4 l_tcp)
{
INTS4 i,l,k,n=0,l_status;
INTU4 *pl_count,*pl_size;

l_status = f_ut_setup_ini(ps_setup);

/*printf("setup Send %d LW + ",ps_setup->l_fix_lw);*/
l_status = f_stc_write (ps_setup, ps_setup->l_fix_lw*4, l_tcp);
pl_count = pl_o++;
pl_size = pl_o++;
l = 0;
for(i=0; i<ps_setup->bl_sbs__n_cr; i++)
{
  if(ps_setup->bh_rd_flg[i] != 0)
  {
    n=0;
    l++;
    *pl_o++ = i; n++;
    *pl_o++ = (INTU4) ps_setup->lp_rem_mem_base[i]; n++;
    *pl_o++ = (INTU4) ps_setup->bl_rem_mem_off[i]; n++;
    *pl_o++ = (INTU4) ps_setup->bl_rem_mem_len[i]; n++;
    *pl_o++ = (INTU4) ps_setup->lp_rem_cam_base[i]; n++;
    *pl_o++ = (INTU4) ps_setup->bl_rem_cam_off[i]; n++;
    *pl_o++ = (INTU4) ps_setup->bl_rem_cam_len[i]; n++;
    *pl_o++ = (INTU4) ps_setup->lp_loc_mem_base[i]; n++;
    *pl_o++ = (INTU4) ps_setup->bl_loc_mem_len[i]; n++;
    *pl_o++ = (INTU4) ps_setup->lp_loc_pipe_base[i]; n++;
    *pl_o++ = (INTU4) ps_setup->bl_pipe_off[i]; n++;
    *pl_o++ = (INTU4) ps_setup->bl_pipe_seg_len[i]; n++;
    *pl_o++ = (INTU4) ps_setup->bl_pipe_len[i]; n++;
    *pl_o++ = (INTU4) ps_setup->bh_controller_id[i]; n++;
    *pl_o++ = (INTU4) ps_setup->bh_sy_asy_flg[i]; n++;
    *pl_o++ = (INTU4) ps_setup->bh_trig_stat_nr[i]; n++;
    *pl_o++ = (INTU4) ps_setup->bl_trig_cvt[i]; n++;
    *pl_o++ = (INTU4) ps_setup->bl_trig_fct[i]; n++;
    *pl_o++ = (INTU4) ps_setup->i_se_typ[i]; n++;
    *pl_o++ = (INTU4) ps_setup->i_se_subtyp[i]; n++;
    *pl_o++ = (INTU4) ps_setup->i_se_procid[i]; n++;
    *pl_o++ = (INTU4) ps_setup->bh_rd_flg[i]; n++;
    *pl_o++ = (INTU4) ps_setup->bl_init_tab_off[i]; n++;
    *pl_o++ = (INTU4) ps_setup->bi_init_tab_len[i]; n++;
    for(k=0;k<ps_setup->bl_sbs__n_trg_typ;k++)
    *pl_o++ = (INTU4) ps_setup->bl_max_se_len[i][k];
    for(k=0;k<ps_setup->bl_sbs__n_trg_typ;k++)
    *pl_o++ = (INTU4) ps_setup->bl_rd_tab_off[i][k];
    for(k=0;k<ps_setup->bl_sbs__n_trg_typ;k++)
    *pl_o++ = (INTU4) ps_setup->bi_rd_tab_len[i][k];
    n = n + 3 * ps_setup->bl_sbs__n_trg_typ;
  }
}
*pl_count=l;
*pl_size=n;
l = l * n + 2;
/*printf("%d \n",l);*/
l_status = f_stc_write (pl_count, l * 4, l_tcp);
return l_status;
}

/*****************+***********+****************************************/
/*                                                                    */
/*   GSI, Gesellschaft fuer Schwerionenforschung mbH                  */
/*   Postfach 11 05 52                                                */
/*   D-64220 Darmstadt                                                */
/*                                                                    */
/*1+ C Procedure *************+****************************************/
/*                                                                    */
/*+ Module      : f_ut_setup_r                                        */
/*                                                                    */
/*--------------------------------------------------------------------*/
/*+ CALLING     : l_status=f_ut_setup_r(s_setup, socket)              */
/*--------------------------------------------------------------------*/
/*                                                                    */
/*+ PURPOSE     : Read s_setup from socket.                           */
/*                                                                    */
/*+ ARGUMENTS   :                                                     */
/*+  s_setup    : pointer to s_setup                                  */
/*+  socket     : Tcp socket from connect                             */
/*2+Implementation************+****************************************/
/*+ File name   : f_ut_status.c                                       */
/*+ Home direct.: /mbs/prod/src                                       */
/*+ Version     : 2.01                                                */
/*+ Author      : H.G.Essel                                           */
/*+ Created     : 14-Nov-1998                                         */
/*+ Object libr.: mbslib.a                                            */
/*+ Updates     : Date        Purpose                                 */
/*1- C Procedure *************+****************************************/
INTS4 f_ut_setup_r(s_setup *ps_setup, INTS4 l_tcp)
{

  /* TODO TODO JAM64 adjust different member length here for 64 bit senders
     * Probably we need architecture independent structure for tcp communication?*/

INTS4 l_swap=0;
INTS4 l_cmd;
INTS4 i,k,l_items,l_size,l_crate;
INTU4 *pl_b,*pl_o;
INTS4 l_status;
s_setup_64_receiver *ps_setup_64=0;
s_setup_32_receiver *ps_setup_32=0;

 memset(ps_setup,0,sizeof(s_setup));
l_cmd=2;
l_status = f_stc_write (&l_cmd,4,l_tcp);                 if (l_status != STC__SUCCESS) return(-1);
l_status = f_stc_read (&ps_setup->l_endian,16,l_tcp,-1); if (l_status != STC__SUCCESS) return(-1);
if(ps_setup->l_endian != 1) l_swap = 1;
if(l_swap == 1) l_status = f_swaplw((INTS4 *) &ps_setup->l_endian,4,NULL);
if((ps_setup->l_version != VERSION__SETUP) && (ps_setup->l_version != 1))
{
  printf ("f_ut_setup_r sees setup version %d, current version is %d", ps_setup->l_version, VERSION__SETUP);
  return -1; // correct for legacy MBS
}

// JAM here decide if setup comes from 32 or 64 bit host:
if(ps_setup->l_fix_lw == 40)
{
  // data from 32 bit machine.  first use auxiliary structure:
   ps_setup_32  = (s_setup_32_receiver*) malloc (sizeof( s_setup_32_receiver));
   if (ps_setup_32 == NULL) {
      printf ("f_ut_setup_r memory allocation error");
      return -1;
   }
   l_status = f_stc_read (&ps_setup_32->bl_sbs__n_cr, (ps_setup->l_fix_lw-4)*4 , l_tcp,-1);
   l_status = f_stc_read (&l_items,4 , l_tcp,-1);
   l_status = f_stc_read (&l_size,4 , l_tcp,-1);
   if(l_swap == 1) l_status = f_swaplw((INTS4 *) &ps_setup_32->bl_sbs__n_cr, (ps_setup->l_fix_lw-4),NULL);
   if(l_swap == 1) l_status = f_swaplw(&l_items,1,NULL);
   if(l_swap == 1) l_status = f_swaplw(&l_size,1,NULL);

   // copy values to actual local setup (which can be both 32 or 64 bit, it should work):
   f_ut_setup_copy32(ps_setup,ps_setup_32);
   free(ps_setup_32);
}
else if(ps_setup->l_fix_lw == 46)
{
  // 64 bit with mbs >v6.3. use auxiliary structure:
  ps_setup_64   = (s_setup_64_receiver*) malloc (sizeof( s_setup_64_receiver));
  if (ps_setup_64 == NULL) {
     printf ("f_ut_setup_r memory allocation error");
     return -1;
  }

  l_status = f_stc_read (&ps_setup_64->bl_sbs__n_cr, (ps_setup->l_fix_lw-4)*4 , l_tcp,-1);
               l_status = f_stc_read (&l_items,4 , l_tcp,-1);
               l_status = f_stc_read (&l_size,4 , l_tcp,-1);
  if(l_swap == 1) l_status = f_swaplw((INTS4 *) &ps_setup_64->bl_sbs__n_cr, (ps_setup->l_fix_lw-4),NULL);
  if(l_swap == 1) l_status = f_swaplw(&l_items,1,NULL);
  if(l_swap == 1) l_status = f_swaplw(&l_size,1,NULL);

  // copy values to actual local setup (which can be both 32 or 64 bit, it should work):
  f_ut_setup_copy64(ps_setup,ps_setup_64);
  free(ps_setup_64);
}
else
{
  printf("f_ut_setup_r finds non consistent l_fix_lw=%d. do not read further!", ps_setup->l_fix_lw);
  return -1;
}

// the rest does not depend on architecture, each word is transferred separately with 4 bytes
// (losing some 64 bit addresses, but anyway now TODO adjust sending of data too):

pl_b = (INTU4 *) malloc(l_size * l_items * 4);
             l_status = f_stc_read (pl_b,l_size * l_items * 4, l_tcp,-1);
if(l_swap == 1) l_status = f_swaplw((INTS4 *) pl_b,l_size * l_items,NULL);

pl_o = pl_b;
for(i=0;i<l_items;i++)
{
    l_crate = *pl_o++;
     ps_setup->lp_rem_mem_base[l_crate] =  *pl_o++;
     ps_setup->bl_rem_mem_off[l_crate] =  *pl_o++;
     ps_setup->bl_rem_mem_len[l_crate] =  *pl_o++;
     ps_setup->lp_rem_cam_base[l_crate] = *pl_o++;
     ps_setup->bl_rem_cam_off[l_crate] = *pl_o++;
     ps_setup->bl_rem_cam_len[l_crate] = *pl_o++;
     ps_setup->lp_loc_mem_base[l_crate] =   *pl_o++;
     ps_setup->bl_loc_mem_len[l_crate] =    *pl_o++;
     ps_setup->lp_loc_pipe_base[l_crate] =  *pl_o++;
     ps_setup->bl_pipe_off[l_crate] =       *pl_o++;
     ps_setup->bl_pipe_seg_len[l_crate] =  *pl_o++;
     ps_setup->bl_pipe_len[l_crate] = *pl_o++;
     ps_setup->bh_controller_id[l_crate] = *pl_o++;
     ps_setup->bh_sy_asy_flg[l_crate] = *pl_o++;
     ps_setup->bh_trig_stat_nr[l_crate] = *pl_o++;
     ps_setup->bl_trig_cvt[l_crate] = *pl_o++;
     ps_setup->bl_trig_fct[l_crate] = *pl_o++;
     ps_setup->i_se_typ[l_crate] = *pl_o++;
     ps_setup->i_se_subtyp[l_crate] = *pl_o++;
     ps_setup->i_se_procid[l_crate] = *pl_o++;
     ps_setup->bh_rd_flg[l_crate] = *pl_o++;
     ps_setup->bl_init_tab_off[l_crate] = *pl_o++;
     ps_setup->bi_init_tab_len[l_crate] = *pl_o++;
    for(k=0;k<ps_setup->bl_sbs__n_trg_typ;k++) ps_setup->bl_max_se_len[l_crate][k] = *pl_o++;
    for(k=0;k<ps_setup->bl_sbs__n_trg_typ;k++) ps_setup->bl_rd_tab_off[l_crate][k] = *pl_o++;
    for(k=0;k<ps_setup->bl_sbs__n_trg_typ;k++) ps_setup->bi_rd_tab_len[l_crate][k] = *pl_o++;
} /* setup */

free(pl_b);
return 0;
}

/*****************+***********+****************************************/
/*                                                                    */
/*   GSI, Gesellschaft fuer Schwerionenforschung mbH                  */
/*   Postfach 11 05 52                                                */
/*   D-64220 Darmstadt                                                */
/*                                                                    */
/*1+ C Procedure *************+****************************************/
/*                                                                    */
/*+ Module      : f_ut_set_ml_ini                                     */
/*                                                                    */
/*--------------------------------------------------------------------*/
/*+ CALLING     : l_status=f_ut_set_ml_ini(s_set_ml)                  */
/*--------------------------------------------------------------------*/
/*                                                                    */
/*+ PURPOSE     : Initialize s_set_ml.                                */
/*                                                                    */
/*+ ARGUMENTS   :                                                     */
/*+  s_set_ml   : pointer to s_set_ml                                 */
/*2+Implementation************+****************************************/
/*+ File name   : f_ut_status.c                                       */
/*+ Home direct.: /mbs/prod/src                                       */
/*+ Version     : 2.01                                                */
/*+ Author      : H.G.Essel                                           */
/*+ Created     : 14-Nov-1998                                         */
/*+ Object libr.: mbslib.a                                            */
/*+ Updates     : Date        Purpose                                 */
/*1- C Procedure *************+****************************************/
INTS4 f_ut_set_ml_ini(s_set_ml *ps_set_ml)
{
if(ps_set_ml == NULL) return -1;
ps_set_ml->l_endian        = 1;
ps_set_ml->l_version       = VERSION__SET_ML;
ps_set_ml->l_set_ml_lw     = sizeof(s_set_ml)/4;
ps_set_ml->l_fix_lw        = ((ADDRS)&ps_set_ml->c_rd_hostname[0][0]-(ADDRS)ps_set_ml)/4;
ps_set_ml->l_ml__n_rd_pipe = ML__N_RD_PIPE;
ps_set_ml->l_short_len     = 16;
ps_set_ml->l_long_len      = 128;
return 0;
}

/*****************+***********+****************************************/
/*                                                                    */
/*   GSI, Gesellschaft fuer Schwerionenforschung mbH                  */
/*   Postfach 11 05 52                                                */
/*   D-64220 Darmstadt                                                */
/*                                                                    */
/*1+ C Procedure *************+****************************************/
/*                                                                    */
/*+ Module      : f_ut_set_ml                                         */
/*                                                                    */
/*--------------------------------------------------------------------*/
/*+ CALLING     : l_status=f_ut_set_ml(s_set_ml, s_set_ml, socket)    */
/*--------------------------------------------------------------------*/
/*                                                                    */
/*+ PURPOSE     : Write s_set_ml to socket.                           */
/*                                                                    */
/*+ ARGUMENTS   :                                                     */
/*+  s_set_ml   : pointer to s_set_ml                                 */
/*+  s_set_ml   : pointer to copy of s_set_ml                         */
/*+  socket     : Tcp socket from accept                              */
/*2+Implementation************+****************************************/
/*+ File name   : f_ut_status.c                                       */
/*+ Home direct.: /mbs/prod/src                                       */
/*+ Version     : 2.01                                                */
/*+ Author      : H.G.Essel                                           */
/*+ Created     : 14-Nov-1998                                         */
/*+ Object libr.: mbslib.a                                            */
/*+ Updates     : Date        Purpose                                 */
/*1- C Procedure *************+****************************************/
INTS4 f_ut_set_ml(s_set_ml *ps_set_ml, INTU4 *pl_o, INTS4 l_tcp)
{
INTS4 i,l,k,l_status;
INTU4 *pl_count,*pl_s;

l_status = f_ut_set_ml_ini(ps_set_ml);

pl_count = pl_o;
/*printf("MLset Send %d LW + ",ps_set_ml->l_fix_lw);*/
l_status = f_stc_write (ps_set_ml, ps_set_ml->l_fix_lw*4, l_tcp);
l = 0;
for(i=0; i<ps_set_ml->l_n_rd_pipe; i++)
{
    l++;
    pl_s = (INTU4 *)&ps_set_ml->c_rd_hostname[i];
    for(k=0;k<ps_set_ml->l_short_len/4;k++)*pl_o++ = *pl_s++;
    pl_s = (INTU4 *)&ps_set_ml->c_sbs_setup_path[i];
    for(k=0;k<ps_set_ml->l_long_len/4;k++)*pl_o++ = *pl_s++;
}
l = l * (ps_set_ml->l_short_len + ps_set_ml->l_long_len);
/*printf("%d \n",l);*/
l_status = f_stc_write (pl_count, l , l_tcp);
return l_status;
}

/*****************+***********+****************************************/
/*                                                                    */
/*   GSI, Gesellschaft fuer Schwerionenforschung mbH                  */
/*   Postfach 11 05 52                                                */
/*   D-64220 Darmstadt                                                */
/*                                                                    */
/*1+ C Procedure *************+****************************************/
/*                                                                    */
/*+ Module      : f_ut_set_ml_r                                       */
/*                                                                    */
/*--------------------------------------------------------------------*/
/*+ CALLING     : l_status=f_ut_set_ml_r(s_set_ml, socket)            */
/*--------------------------------------------------------------------*/
/*                                                                    */
/*+ PURPOSE     : Read s_set_ml from socket.                          */
/*                                                                    */
/*+ ARGUMENTS   :                                                     */
/*+  s_set_ml   : pointer to s_set_ml                                 */
/*+  socket     : Tcp socket from connect                             */
/*2+Implementation************+****************************************/
/*+ File name   : f_ut_status.c                                       */
/*+ Home direct.: /mbs/prod/src                                       */
/*+ Version     : 2.01                                                */
/*+ Author      : H.G.Essel                                           */
/*+ Created     : 14-Nov-1998                                         */
/*+ Object libr.: mbslib.a                                            */
/*+ Updates     : Date        Purpose                                 */
/*1- C Procedure *************+****************************************/
INTS4 f_ut_set_ml_r(s_set_ml *ps_set_ml, INTS4 l_tcp)
{
INTS4 l_swap=0;
INTS4 l_cmd;
INTS4 i;
INTS4 l_status;

l_cmd=3;
l_status = f_stc_write (&l_cmd,4, l_tcp);                 if (l_status != STC__SUCCESS) return(-1);
l_status = f_stc_read (&ps_set_ml->l_endian,16,l_tcp,-1); if (l_status != STC__SUCCESS) return(-1);
if(ps_set_ml->l_endian != 1) l_swap = 1;
if(l_swap == 1) l_status = f_swaplw((INTS4 *)&ps_set_ml->l_endian,4,NULL);
if( (ps_set_ml->l_version != VERSION__SET_ML) && (ps_set_ml->l_version != 1)) return -1; // correct legacy MBS version
             l_status = f_stc_read (&ps_set_ml->l_ml__n_rd_pipe,(ps_set_ml->l_fix_lw-4)*4 , l_tcp,-1);
if(l_swap == 1) l_status = f_swaplw((INTS4 *)&ps_set_ml->l_ml__n_rd_pipe,(ps_set_ml->l_fix_lw-4)-4,NULL); /* last 16 byte are char */
for(i=0;i<ps_set_ml->l_n_rd_pipe;i++)
{
   l_status = f_stc_read (&ps_set_ml->c_rd_hostname[i],   ps_set_ml->l_short_len, l_tcp,-1);
   l_status = f_stc_read (&ps_set_ml->c_sbs_setup_path[i],ps_set_ml->l_long_len,  l_tcp,-1);
}
return 0;
}
/*****************+***********+****************************************/
/*                                                                    */
/*   GSI, Gesellschaft fuer Schwerionenforschung mbH                  */
/*   Postfach 11 05 52                                                */
/*   D-64220 Darmstadt                                                */
/*                                                                    */
/*1+ C Procedure *************+****************************************/
/*                                                                    */
/*+ Module      : f_ut_set_mo_ini                                     */
/*                                                                    */
/*--------------------------------------------------------------------*/
/*+ CALLING     : l_status=f_ut_set_mo_ini(s_set_mo)                  */
/*--------------------------------------------------------------------*/
/*                                                                    */
/*+ PURPOSE     : Initialize s_set_mo.                                */
/*                                                                    */
/*+ ARGUMENTS   :                                                     */
/*+  s_set_mo   : pointer to s_set_mo                                 */
/*2+Implementation************+****************************************/
/*+ File name   : f_ut_status.c                                       */
/*+ Home direct.: /mbs/prod/src                                       */
/*+ Version     : 2.01                                                */
/*+ Author      : H.G.Essel                                           */
/*+ Created     : 24-Oct-2005                                         */
/*+ Object libr.: mbslib.a                                            */
/*+ Updates     : Date        Purpose                                 */
/*1- C Procedure *************+****************************************/
INTS4 f_ut_set_mo_ini(s_set_mo *ps_set_mo)
{
  INTS4 i;
if(ps_set_mo == NULL) return -1;
ps_set_mo->l_endian        = 1;
ps_set_mo->l_version       = VERSION__SET_MO;
ps_set_mo->l_set_mo_lw     = sizeof(s_set_mo)/4;
ps_set_mo->l_swap_lw       = ((ADDRS)&ps_set_mo->c_ds_hostname[0][0]-(ADDRS)ps_set_mo)/4;
ps_set_mo->l_max_nodes     = MO__N_NODE;
ps_set_mo->l_no_senders    = 0;
ps_set_mo->l_no_receivers  = 0;
 for(i=0;i<MO__N_NODE;i++)if(strlen(&ps_set_mo->c_ds_hostname[i][0]) > 0) ps_set_mo->l_no_senders++;
 for(i=0;i<MO__N_NODE;i++)if(strlen(&ps_set_mo->c_dr_hostname[i][0]) > 0) ps_set_mo->l_no_receivers++;
return 0;
}
/*1+ C Procedure *************+****************************************/
/*                                                                    */
/*+ Module      : f_ut_set_mo                                         */
/*                                                                    */
/*--------------------------------------------------------------------*/
/*+ CALLING     : l_status=f_ut_set_mo(s_set_mo, socket)               */
/*--------------------------------------------------------------------*/
/*                                                                    */
/*+ PURPOSE     : Send s_set_mo to socket.                             */
/*                                                                    */
/*+ ARGUMENTS   :                                                     */
/*+  s_daqst    : pointer to s_set_mo                                  */
/*+  socket     : Tcp socket from accept                              */
/*2+Implementation************+****************************************/
/*+ File name   : f_ut_status.c                                       */
/*+ Home direct.: /mbs/prod/src                                       */
/*+ Version     : 2.01                                                */
/*+ Author      : H.G.Essel                                           */
/*+ Created     : 25.Oct.2005                                         */
/*+ Object libr.: mbslib.a                                            */
/*+ Updates     : Date        Purpose                                 */
/*1- C Procedure *************+****************************************/
INTS4 f_ut_set_mo(s_set_mo *ps_set_mo, INTS4 l_tcp)
{
   INTS4 l_status;

   l_status = f_ut_set_mo_ini(ps_set_mo);
   if (l_status != 0)
      return l_status;
   l_status = f_stc_write(ps_set_mo, ps_set_mo->l_set_mo_lw * 4, l_tcp);
   return l_status;
}


/*1+ C Procedure *************+****************************************/
/*                                                                    */
/*+ Module      : f_ut_set_mo_r                                       */
/*                                                                    */
/*--------------------------------------------------------------------*/
/*+ CALLING     : l_status=f_ut_set_mo_r(s_set_mo, socket)            */
/*--------------------------------------------------------------------*/
/*                                                                    */
/*+ PURPOSE     : Read s_set_mo from socket.                          */
/*                                                                    */
/*+ ARGUMENTS   :                                                     */
/*+  s_set_mo   : pointer to s_set_mo                                 */
/*+  socket     : Tcp socket from connect                             */
/*2+Implementation************+****************************************/
/*+ File name   : f_ut_status.c                                       */
/*+ Home direct.: /mbs/prod/src                                       */
/*+ Version     : 2.01                                                */
/*+ Author      : H.G.Essel                                           */
/*+ Created     : 25.Oct.2005                                         */
/*+ Object libr.: mbslib.a                                            */
/*+ Updates     : Date        Purpose                                 */
/*1- C Procedure *************+****************************************/
INTS4 f_ut_set_mo_r(s_set_mo *ps_set_mo, INTS4 l_tcp)
{
INTS4 l_swap=0;
INTS4 l_cmd;
INTS4 l_status;

l_cmd=4;
l_status = f_stc_write (&l_cmd,4, l_tcp);  if (l_status != STC__SUCCESS) return(-1);
l_status = f_stc_read (ps_set_mo,16,l_tcp,-1); if (l_status != STC__SUCCESS) return(-1);
if(ps_set_mo->l_endian != 1) l_swap=1;
if(l_swap) l_status = f_swaplw((INTS4*)ps_set_mo,4,NULL);

l_status = f_stc_read (&ps_set_mo->l_max_nodes,(ps_set_mo->l_set_mo_lw-4)*4,  l_tcp,-1);
if(l_swap) l_status = f_swaplw((INTS4*)&ps_set_mo->l_max_nodes,ps_set_mo->l_swap_lw-4,NULL);

return 0;
}
