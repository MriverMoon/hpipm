/**************************************************************************************************
*                                                                                                 *
* This file is part of HPIPM.                                                                     *
*                                                                                                 *
* HPIPM -- High Performance Interior Point Method.                                                *
* Copyright (C) 2017 by Gianluca Frison.                                                          *
* Developed at IMTEK (University of Freiburg) under the supervision of Moritz Diehl.              *
* All rights reserved.                                                                            *
*                                                                                                 *
* HPIPM is free software; you can redistribute it and/or                                          *
* modify it under the terms of the GNU Lesser General Public                                      *
* License as published by the Free Software Foundation; either                                    *
* version 2.1 of the License, or (at your option) any later version.                              *
*                                                                                                 *
* HPIPM is distributed in the hope that it will be useful,                                        *
* but WITHOUT ANY WARRANTY; without even the implied warranty of                                  *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                                            *
* See the GNU Lesser General Public License for more details.                                     *
*                                                                                                 *
* You should have received a copy of the GNU Lesser General Public                                *
* License along with HPIPM; if not, write to the Free Software                                    *
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA                  *
*                                                                                                 *
* Author: Gianluca Frison, gianluca.frison (at) imtek.uni-freiburg.de                             *
*                                                                                                 *
**************************************************************************************************/



int MEMSIZE_IPM_HARD_DENSE_QP(struct DENSE_QP *qp, struct IPM_HARD_DENSE_QP_ARG *arg)
	{

	int nv = qp->nv;
	int ne = qp->ne;
	int nb = qp->nb;
	int ng = qp->ng;
	int ns = qp->ns;

	int size = 0;

	size += 15*sizeof(struct STRVEC); // dv dpi dlam dt res_g res_b res_d res_m lv 2*tmp_nbg tmp_ns Gamma gamma Zs_inv
	size += 4*sizeof(struct STRMAT); // Lv AL Le Ctx

	size += 2*SIZE_STRVEC(nb+ng); // 2*tmp_nbg
	size += 1*SIZE_STRVEC(ns); // tmp_ns
	size += 1*SIZE_STRVEC(nv); // lv
	size += 1*SIZE_STRVEC(2*ns); // Zs_inv
	size += 1*SIZE_STRMAT(nv+1, nv); // Lv
	size += 1*SIZE_STRMAT(ne, nv); // AL
	size += 1*SIZE_STRMAT(ne, ne); // Le
	size += 1*SIZE_STRMAT(nv+1, ng); // Ctx

	size += 1*sizeof(struct IPM_CORE_QP_WORKSPACE);
	size += 1*MEMSIZE_IPM_CORE_QP(nv, ne, nb+ng, arg->iter_max); // XXX

	size = (size+63)/64*64; // make multiple of typical cache line size
	size += 1*64; // align once to typical cache line size

	return size;

	}



void CREATE_IPM_HARD_DENSE_QP(struct DENSE_QP *qp, struct IPM_HARD_DENSE_QP_ARG *arg, struct IPM_HARD_DENSE_QP_WORKSPACE *workspace, void *mem)
	{

	workspace->memsize = MEMSIZE_IPM_HARD_DENSE_QP(qp, arg);


	int nv = qp->nv;
	int ne = qp->ne;
	int nb = qp->nb;
	int ng = qp->ng;
	int ns = qp->ns;


	// core struct
	struct IPM_CORE_QP_WORKSPACE *sr_ptr = mem;

	// core workspace
	workspace->core_workspace = sr_ptr;
	sr_ptr += 1;
	struct IPM_CORE_QP_WORKSPACE *cws = workspace->core_workspace;


	// matrix struct
	struct STRMAT *sm_ptr = (struct STRMAT *) sr_ptr;

	workspace->Lv = sm_ptr;
	sm_ptr += 1;
	workspace->AL = sm_ptr;
	sm_ptr += 1;
	workspace->Le = sm_ptr;
	sm_ptr += 1;
	workspace->Ctx = sm_ptr;
	sm_ptr += 1;


	// vector struct
	struct STRVEC *sv_ptr = (struct STRVEC *) sm_ptr;

	workspace->dv = sv_ptr;
	sv_ptr += 1;
	workspace->dpi = sv_ptr;
	sv_ptr += 1;
	workspace->dlam = sv_ptr;
	sv_ptr += 1;
	workspace->dt = sv_ptr;
	sv_ptr += 1;
	workspace->res_g = sv_ptr;
	sv_ptr += 1;
	workspace->res_b = sv_ptr;
	sv_ptr += 1;
	workspace->res_d = sv_ptr;
	sv_ptr += 1;
	workspace->res_m = sv_ptr;
	sv_ptr += 1;
	workspace->Gamma = sv_ptr;
	sv_ptr += 1;
	workspace->gamma = sv_ptr;
	sv_ptr += 1;
	workspace->Zs_inv = sv_ptr;
	sv_ptr += 1;
	workspace->lv = sv_ptr;
	sv_ptr += 1;
	workspace->tmp_nbg = sv_ptr;
	sv_ptr += 2;
	workspace->tmp_ns = sv_ptr;
	sv_ptr += 1;


	// align to typicl cache line size
	size_t s_ptr = (size_t) sv_ptr;
	s_ptr = (s_ptr+63)/64*64;


	// void stuf
	char *c_ptr = (char *) s_ptr;

	CREATE_STRMAT(nv+1, nv, workspace->Lv, c_ptr);
	c_ptr += workspace->Lv->memory_size;

	CREATE_STRMAT(ne, nv, workspace->AL, c_ptr);
	c_ptr += workspace->AL->memory_size;

	CREATE_STRMAT(ne, ne, workspace->Le, c_ptr);
	c_ptr += workspace->Le->memory_size;

	CREATE_STRMAT(nv+1, ng, workspace->Ctx, c_ptr);
	c_ptr += workspace->Ctx->memory_size;

	CREATE_STRVEC(nv, workspace->lv, c_ptr);
	c_ptr += workspace->lv->memory_size;

	CREATE_STRVEC(2*ns, workspace->Zs_inv, c_ptr);
	c_ptr += workspace->Zs_inv->memory_size;

	CREATE_STRVEC(nb+ng, workspace->tmp_nbg+0, c_ptr);
	c_ptr += (workspace->tmp_nbg+0)->memory_size;

	CREATE_STRVEC(nb+ng, workspace->tmp_nbg+1, c_ptr);
	c_ptr += (workspace->tmp_nbg+1)->memory_size;

	CREATE_STRVEC(nb+ng, workspace->tmp_ns+0, c_ptr);
	c_ptr += (workspace->tmp_ns+0)->memory_size;

	cws->nv = nv+2*ns;
	cws->ne = ne;
	cws->nc = nb+ng+ns; // XXX
	cws->iter_max = arg->iter_max;
	CREATE_IPM_CORE_QP(cws, c_ptr);
	c_ptr += workspace->core_workspace->memsize;

	cws->alpha_min = arg->alpha_min;
	cws->mu_max = arg->mu_max;
	cws->mu0 = arg->mu0;
	cws->nt_inv = 1.0/(2*nb+2*ng);


	// alias members of workspace and core_workspace
	CREATE_STRVEC(nv+2*ns, workspace->dv, cws->dv);
	CREATE_STRVEC(ne, workspace->dpi, cws->dpi);
	CREATE_STRVEC(2*nb+2*ng+2*ns, workspace->dlam, cws->dlam);
	CREATE_STRVEC(2*nb+2*ng+2*ns, workspace->dt, cws->dt);
	CREATE_STRVEC(nv+2*ns, workspace->res_g, cws->res_g);
	CREATE_STRVEC(ne, workspace->res_b, cws->res_b);
	CREATE_STRVEC(2*nb+2*ng+2*ns, workspace->res_d, cws->res_d);
	CREATE_STRVEC(2*nb+2*ng+2*ns, workspace->res_m, cws->res_m);
	CREATE_STRVEC(2*nb+2*ng+2*ns, workspace->Gamma, cws->Gamma);
	CREATE_STRVEC(2*nb+2*ng+2*ns, workspace->gamma, cws->gamma);
	workspace->stat = cws->stat;

	return;

	}



void SOLVE_IPM_HARD_DENSE_QP(struct DENSE_QP *qp, struct DENSE_QP_SOL *qp_sol, struct IPM_HARD_DENSE_QP_WORKSPACE *ws)
	{

	struct IPM_CORE_QP_WORKSPACE *cws = ws->core_workspace;

	// alias qp vectors into qp_sol
	cws->v = qp_sol->v->pa;
	cws->pi = qp_sol->pi->pa;
	cws->lam = qp_sol->lam->pa;
	cws->t = qp_sol->t->pa;

	if(cws->nc==0)
		{
		FACT_SOLVE_KKT_UNCONSTR_DENSE_QP(qp, qp_sol, ws);
		COMPUTE_RES_HARD_DENSE_QP(qp, qp_sol, ws);
		cws->mu = ws->res_mu;
		ws->iter = 0;
		return;
		}

	// init solver
	INIT_VAR_HARD_DENSE_QP(qp, qp_sol, ws);

	// compute residuals
	COMPUTE_RES_HARD_DENSE_QP(qp, qp_sol, ws);
	cws->mu = ws->res_mu;

	int kk;
	for(kk=0; kk<cws->iter_max & cws->mu>cws->mu_max; kk++)
		{

		// fact and solve kkt
		FACT_SOLVE_KKT_STEP_HARD_DENSE_QP(qp, ws);

		// alpha
		COMPUTE_ALPHA_QP(cws);
		cws->stat[5*kk+0] = cws->alpha;

		//
		UPDATE_VAR_QP(cws);

		// compute residuals
		COMPUTE_RES_HARD_DENSE_QP(qp, qp_sol, ws);
		cws->mu = ws->res_mu;
		cws->stat[5*kk+1] = ws->res_mu;

		}

	ws->iter = kk;

	return;

	}



void SOLVE_IPM2_HARD_DENSE_QP(struct DENSE_QP *qp, struct DENSE_QP_SOL *qp_sol, struct IPM_HARD_DENSE_QP_WORKSPACE *ws)
	{

	struct IPM_CORE_QP_WORKSPACE *cws = ws->core_workspace;

	// alias qp vectors into qp_sol
	cws->v = qp_sol->v->pa;
	cws->pi = qp_sol->pi->pa;
	cws->lam = qp_sol->lam->pa;
	cws->t = qp_sol->t->pa;

	REAL tmp;

	if(cws->nc==0)
		{
		FACT_SOLVE_KKT_UNCONSTR_DENSE_QP(qp, qp_sol, ws);
		COMPUTE_RES_HARD_DENSE_QP(qp, qp_sol, ws);
		cws->mu = ws->res_mu;
		ws->iter = 0;
		return;
		}

	// init solver
	INIT_VAR_HARD_DENSE_QP(qp, qp_sol, ws);

	// compute residuals
	COMPUTE_RES_HARD_DENSE_QP(qp, qp_sol, ws);
	cws->mu = ws->res_mu;

	int kk;
	for(kk=0; kk<cws->iter_max & cws->mu>cws->mu_max; kk++)
		{

		// fact and solve kkt
		FACT_SOLVE_KKT_STEP_HARD_DENSE_QP(qp, ws);

		// alpha
		COMPUTE_ALPHA_QP(cws);
		cws->stat[5*kk+0] = cws->alpha;

		// mu_aff
		COMPUTE_MU_AFF_QP(cws);
		cws->stat[5*kk+1] = cws->mu_aff;

		tmp = cws->mu_aff/cws->mu;
		cws->sigma = tmp*tmp*tmp;
		cws->stat[5*kk+2] = cws->sigma;

		COMPUTE_CENTERING_CORRECTION_QP(cws);

		// fact and solve kkt
		SOLVE_KKT_STEP_HARD_DENSE_QP(qp, ws);

		// alpha
		COMPUTE_ALPHA_QP(cws);
		cws->stat[5*kk+3] = cws->alpha;

		//
		UPDATE_VAR_QP(cws);

		// compute residuals
		COMPUTE_RES_HARD_DENSE_QP(qp, qp_sol, ws);
		cws->mu = ws->res_mu;
		cws->stat[5*kk+4] = ws->res_mu;

		}

	ws->iter = kk;

	return;

	}

