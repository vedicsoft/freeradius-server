/*
 * cryptocard.h
 * $Id$
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Copyright 2005 Frank Cusack
 */

#ifndef CRYPTOCARD_H
#define CRYPTOCARD_H

#include "../x99_cardops.h"

/* card modes */
#define CRYPTOCARD_H8_RC (X99_CF_HD|X99_CF_R8|X99_CF_AM)
#define CRYPTOCARD_H7_RC (X99_CF_HD|X99_CF_R7|X99_CF_AM)
#define CRYPTOCARD_D8_RC (X99_CF_DD|X99_CF_R8|X99_CF_AM)
#define CRYPTOCARD_D7_RC (X99_CF_DD|X99_CF_R7|X99_CF_AM)
#define CRYPTOCARD_H8_ES (X99_CF_HD|X99_CF_R8|X99_CF_ES)
#define CRYPTOCARD_H7_ES (X99_CF_HD|X99_CF_R7|X99_CF_ES)
#define CRYPTOCARD_D8_ES (X99_CF_DD|X99_CF_R8|X99_CF_ES)
#define CRYPTOCARD_D7_ES (X99_CF_DD|X99_CF_R7|X99_CF_ES)
#define CRYPTOCARD_H8_RS (CRYPTOCARD_H8_RC|CRYPTOCARD_H8_ES)
#define CRYPTOCARD_H7_RS (CRYPTOCARD_H7_RC|CRYPTOCARD_H7_ES)
#define CRYPTOCARD_D8_RS (CRYPTOCARD_D8_RC|CRYPTOCARD_D8_ES)
#define CRYPTOCARD_D7_RS (CRYPTOCARD_D7_RC|CRYPTOCARD_D7_ES)

static int cryptocard_name2fm(const char *name, uint32_t *featuremask);
static int cryptocard_keystring2keyblock(const char *keystring,
					 unsigned char keyblock[]);
static int cryptocard_challenge(const char *syncdir,
				x99_user_info_t *user_info,
				int ewin, int twin,
				char challenge[MAX_CHALLENGE_LEN + 1]);
static int cryptocard_response(x99_user_info_t *user_info,
			       const char *challenge,
			       char response[X99_MAX_RESPONSE_LEN + 1]);

#ifdef __GNUC__
__attribute__ ((constructor))
#endif
void cryptocard_init(void);

#endif /* CRYPTOCARD_H */
