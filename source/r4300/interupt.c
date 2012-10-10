/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - interupt.c                                              *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2002 Hacktarux                                          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <stdlib.h>

#include <debug.h>

#define M64P_CORE_PROTOTYPES 1
#include "api/m64p_types.h"
#include "api/callbacks.h"
#include "memory/memory.h"
#include "main/rom.h"
#include "main/main.h"
#include "main/savestates.h"
#include "plugin/plugin.h"

#include "interupt.h"
#include "r4300.h"
#include "macros.h"
#include "exception.h"
#include "ppc/Wrappers.h"

int vi_field = 0;
unsigned int next_vi = 0;
static int vi_counter = 0;

int interupt_unsafe_state = 0;

typedef struct _interupt_queue
{
	int type;
	unsigned int count;
	struct _interupt_queue *next;
} interupt_queue;

static interupt_queue *q = NULL;

static void clear_queue(void)
{
	while (q != NULL)
	{
		interupt_queue *aux = q->next;
		free(q);
		q = aux;
	}
}

/*static void print_queue(void)
{
	interupt_queue *aux;
	//if (Count < 0x7000000) return;
	DebugMessage(M64MSG_INFO, "------------------ 0x%x", (unsigned int)Count);
	aux = q;
	while (aux != NULL)
	{
		DebugMessage(M64MSG_INFO, "Count:%x, %x", (unsigned int)aux->count, aux->type);
		aux = aux->next;
	}
}*/

static int SPECIAL_done = 0;

static int before_event(unsigned int evt1, unsigned int evt2, int type2)
{
	if (evt1 - Count < 0x80000000)
	{
		if (evt2 - Count < 0x80000000)
		{
			if ((evt1 - Count) < (evt2 - Count)) return 1;
			else return 0;
		}
		else
		{
			if ((Count - evt2) < 0x10000000)
			{
				switch (type2)
				{
				case SPECIAL_INT:
					if (SPECIAL_done) return 1;
					else return 0;
					break;
				default:
					return 0;
				}
			}
			else return 1;
		}
	}
	else return 0;
}

void add_interupt_event(int type, unsigned int delay)
{
	unsigned int count = Count + delay/**2*/;
	int special = 0;
	interupt_queue *aux = q;

	if (type == SPECIAL_INT /*|| type == COMPARE_INT*/) special = 1;
	if (Count > 0x80000000) SPECIAL_done = 0;

	if (get_event(type))
	{
		DebugMessage(M64MSG_WARNING, "two events of type 0x%x in interrupt queue", type);
	}

	if (q == NULL)
	{
		q = (interupt_queue *) malloc(sizeof (interupt_queue));
		q->next = NULL;
		q->count = count;
		q->type = type;
		next_interupt = q->count;
		//print_queue();
		return;
	}

	if (before_event(count, q->count, q->type) && !special)
	{
		q = (interupt_queue *) malloc(sizeof (interupt_queue));
		q->next = aux;
		q->count = count;
		q->type = type;
		next_interupt = q->count;
		//print_queue();
		return;
	}

	while (aux->next != NULL && (!before_event(count, aux->next->count, aux->next->type) || special))
		aux = aux->next;

	if (aux->next == NULL)
	{
		aux->next = (interupt_queue *) malloc(sizeof (interupt_queue));
		aux = aux->next;
		aux->next = NULL;
		aux->count = count;
		aux->type = type;
	}
	else
	{
		interupt_queue *aux2;
		if (type != SPECIAL_INT)
			while (aux->next != NULL && aux->next->count == count)
				aux = aux->next;
		aux2 = aux->next;
		aux->next = (interupt_queue *) malloc(sizeof (interupt_queue));
		aux = aux->next;
		aux->next = aux2;
		aux->count = count;
		aux->type = type;
	}
}

void add_interupt_event_count(int type, unsigned int count)
{
	add_interupt_event(type, (count - Count)/*/2*/);
}

static void remove_interupt_event(void)
{
	interupt_queue *aux = q->next;
	if (q->type == SPECIAL_INT) SPECIAL_done = 1;
	free(q);
	q = aux;
	if (q != NULL && (q->count > Count || (Count - q->count) < 0x80000000))
		next_interupt = q->count;
	else
		next_interupt = 0;
}

unsigned int get_event(int type)
{
	interupt_queue *aux = q;
	if (q == NULL) return 0;
	if (q->type == type)
		return q->count;
	while (aux->next != NULL && aux->next->type != type)
		aux = aux->next;
	if (aux->next != NULL)
		return aux->next->count;
	return 0;
}

int get_next_event_type(void)
{
	if (q == NULL) return 0;
	return q->type;
}

void remove_event(int type)
{
	interupt_queue *aux = q;
	if (q == NULL) return;
	if (q->type == type)
	{
		aux = aux->next;
		free(q);
		q = aux;
		return;
	}
	while (aux->next != NULL && aux->next->type != type)
		aux = aux->next;
	if (aux->next != NULL) // it's a type int
	{
		interupt_queue *aux2 = aux->next->next;
		free(aux->next);
		aux->next = aux2;
	}
}

void translate_event_queue(unsigned int base)
{
	interupt_queue *aux;
	remove_event(COMPARE_INT);
	remove_event(SPECIAL_INT);
	aux = q;
	while (aux != NULL)
	{
		aux->count = (aux->count - Count) + base;
		aux = aux->next;
	}
	add_interupt_event_count(COMPARE_INT, Compare);
	add_interupt_event_count(SPECIAL_INT, 0);
}

int save_eventqueue_infos(char *buf)
{
	int len = 0;
	interupt_queue *aux = q;
	if (q == NULL)
	{
		*((unsigned int*) &buf[0]) = 0xFFFFFFFF;
		return 4;
	}
	while (aux != NULL)
	{
		memcpy(buf + len, &aux->type, 4);
		memcpy(buf + len + 4, &aux->count, 4);
		len += 8;
		aux = aux->next;
	}
	*((unsigned int*) &buf[len]) = 0xFFFFFFFF;
	return len + 4;
}

void load_eventqueue_infos(char *buf)
{
	int len = 0;
	clear_queue();
	while (*((unsigned int*) &buf[len]) != 0xFFFFFFFF)
	{
		int type = *((unsigned int*) &buf[len]);
		unsigned int count = *((unsigned int*) &buf[len + 4]);
		add_interupt_event_count(type, count);
		len += 8;
	}
}

void init_interupt(void)
{
	SPECIAL_done = 1;
	next_vi = next_interupt = 5000;
	vi_register.vi_delay = next_vi;
	vi_field = 0;
	clear_queue();
	add_interupt_event_count(VI_INT, next_vi);
	add_interupt_event_count(SPECIAL_INT, 0);
}

// Just wrapping up some common code

int chk_status(int chk)
{
	if (chk)
	{
		if (MI_register.mi_intr_reg & MI_register.mi_intr_mask_reg)
		{
			Cause = (Cause | 0x400) & 0xFFFFFF83;
		}
		else
		{
			return 0;
		}
	}
	if ((Status & 7) != 1)
	{
		return 0;
	}
	if (!(Status & Cause & 0xFF00))
	{
		return 0;
	}
	return 1;
}

void check_interupt(void)
{
	if (MI_register.mi_intr_reg & MI_register.mi_intr_mask_reg)
		Cause = (Cause | 0x400) & 0xFFFFFF83;
	else
		Cause &= ~0x400;
	if ((Status & 7) != 1) return;
	if (Status & Cause & 0xFF00)
	{
		if (q == NULL)
		{
			q = (interupt_queue *) malloc(sizeof (interupt_queue));
			q->next = NULL;
			q->count = Count;
			q->type = CHECK_INT;
		}
		else
		{
			interupt_queue* aux = (interupt_queue *) malloc(sizeof (interupt_queue));
			aux->next = q;
			aux->count = Count;
			aux->type = CHECK_INT;
			q = aux;
		}
		next_interupt = Count;
	}
}

void gen_interupt(void)
{
	if (stop == 1)
	{
		vi_counter = 0; // debug
		dyna_stop();
	}

	if (!interupt_unsafe_state)
	{
		if (savestates_get_job() == savestates_job_load)
		{
			savestates_load();
			return;
		}
	}

	if (skip_jump)
	{
		unsigned int dest = skip_jump;
		skip_jump = 0;

		if (q->count > Count || (Count - q->count) < 0x80000000)
			next_interupt = q->count;
		else
			next_interupt = 0;

		interp_addr = dest;
		last_addr = interp_addr;

		return;
	}

	switch (q->type)
	{
	case SPECIAL_INT:
		if (Count > 0x10000000) return;
		remove_interupt_event();
		add_interupt_event_count(SPECIAL_INT, 0);
		return;
		break;
	case VI_INT:
		gfx.updateScreen();
		refresh_stat();

		new_vi();
		if (vi_register.vi_v_sync == 0) vi_register.vi_delay = 500000;
		else vi_register.vi_delay = ((vi_register.vi_v_sync + 1)*1500);
		next_vi += vi_register.vi_delay;
		if (vi_register.vi_status & 0x40) vi_field = 1 - vi_field;
		else vi_field = 0;

		remove_interupt_event();
		add_interupt_event_count(VI_INT, next_vi);

		MI_register.mi_intr_reg |= 0x08;
		if (!chk_status(1))
		{
			return;
		}
		break;

	case COMPARE_INT:
		remove_interupt_event();
		Count += 2;
		add_interupt_event_count(COMPARE_INT, Compare);
		Count -= 2;

		Cause = (Cause | 0x8000) & 0xFFFFFF83;
		if (!chk_status(0))
		{
			return;
		}
		break;

	case CHECK_INT:
		remove_interupt_event();
		break;

	case SI_INT:
		PIF_RAMb[0x3F] = 0x0;
		remove_interupt_event();
		MI_register.mi_intr_reg |= 0x02;
		si_register.si_stat |= 0x1000;
		si_register.si_stat &= ~0x1;
		if (!chk_status(1))
		{
			return;
		}
		break;

	case PI_INT:
		remove_interupt_event();
		MI_register.mi_intr_reg |= 0x10;
		pi_register.read_pi_status_reg &= ~3;
		if (!chk_status(1))
		{
			return;
		}
		break;

	case AI_INT:
		if (ai_register.ai_status & 0x80000000) // full
		{
			unsigned int ai_event = get_event(AI_INT);
			remove_interupt_event();
			ai_register.ai_status &= ~0x80000000;
			ai_register.current_delay = ai_register.next_delay;
			ai_register.current_len = ai_register.next_len;
			add_interupt_event_count(AI_INT, ai_event + ai_register.next_delay);
		}
		else
		{
			remove_interupt_event();
			ai_register.ai_status &= ~0x40000000;
		}
		MI_register.mi_intr_reg |= 0x04;
		if (!chk_status(1))
		{
			return;
		}
		break;

	case SP_INT:
		remove_interupt_event();
		sp_register.sp_status_reg |= 0x203;
		// sp_register.sp_status_reg |= 0x303;

		if (!(sp_register.sp_status_reg & 0x40)) return; // !intr_on_break

		MI_register.mi_intr_reg |= 0x01;
		if (!chk_status(1))
		{
			return;
		}
		break;

	case DP_INT:
		remove_interupt_event();
		dpc_register.dpc_status &= ~2;
		dpc_register.dpc_status |= 0x81;
		MI_register.mi_intr_reg |= 0x20;

		if (!chk_status(1))
		{
			return;
		}
		break;

	default:
		DebugMessage(M64MSG_ERROR, "Unknown interrupt queue event type %.8X.", q->type);
		remove_interupt_event();
		break;
	}

	exception_general();

	if (!interupt_unsafe_state)
	{
		if (savestates_get_job() == savestates_job_save)
		{
			savestates_save();
			return;
		}
	}
}

