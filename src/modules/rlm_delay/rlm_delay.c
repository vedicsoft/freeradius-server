/*
 *   This program is is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or (at
 *   your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

/**
 * $Id$
 * @file rlm_delay.c
 * @brief Add an artificial delay to requests.
 *
 * @copyright 2016 The FreeRADIUS server project
 * @copyright 2016 Arran Cudbard-Bell <a.cudbardb@freeradius.org>
 */
RCSID("$Id$")

#include <freeradius-devel/radiusd.h>
#include <freeradius-devel/modules.h>
#include <freeradius-devel/rad_assert.h>

#include <freeradius-devel/map_proc.h>

typedef struct rlm_delay_t {
	vp_tmpl_t	*delay;			//!< How long we delay for.
	bool		relative;		//!< Whether the delay is relative to the start of request processing.
	bool		force_reschedule;	//!< Whether we should force rescheduling of the request.
} rlm_delay_t;

/*
 *	A mapping of configuration file names to internal variables.
 */
static const CONF_PARSER module_config[] = {
	{ FR_CONF_OFFSET("delay", PW_TYPE_TMPL, rlm_delay_t, delay) },
	{ FR_CONF_OFFSET("relative", PW_TYPE_BOOLEAN, rlm_delay_t, relative), .dflt = "no" },
	{ FR_CONF_OFFSET("force_reschedule", PW_TYPE_BOOLEAN, rlm_delay_t, force_reschedule), .dflt = "no" },
	CONF_PARSER_TERMINATOR
};

static rlm_rcode_t delay_return(UNUSED REQUEST *request, UNUSED void *module_instance, UNUSED void *ctx)
{
	return RLM_MODULE_OK;
}

/** Called when the timeout has expired
 *
 * Marks the request as resumable, and prints the actual delay time.
 */
static void delay_done(REQUEST *request, UNUSED void *module_instance,
		       void *ctx, struct timeval *timeout)
{
	struct timeval *when = talloc_get_type_abort(ctx, struct timeval);

	/*
	 *	Print how long the delay *actually* was.
	 */
	if (RDEBUG_ENABLED3) {
		struct timeval actual;

		fr_timeval_subtract(&actual, timeout, when);

		RDEBUG3("Request delayed by %"PRIu64".%06"PRIu64"s",
			(uint64_t)actual.tv_sec, (uint64_t)actual.tv_usec);
	}

	talloc_free(when);
	unlang_resumable(request);
}

static rlm_rcode_t delay_add(rlm_delay_t *inst, REQUEST *request)
{
	struct timeval	delay;
	struct timeval	*now;
	struct timeval	when;
	int cmp;

	if (tmpl_aexpand(request, &delay, request, inst->delay, NULL, NULL) < 0) return RLM_MODULE_FAIL;

	/*
	 *	Delay is zero (and reschedule is not forced)
	 */
	if (!inst->force_reschedule && (delay.tv_sec == 0) && (delay.tv_usec == 0)) return RLM_MODULE_NOOP;

	MEM(now = talloc(request, struct timeval));
	if (gettimeofday(now, NULL) < 0) {
		REDEBUG("Failed getting current time: %s", fr_syserror(errno));
		return RLM_MODULE_FAIL;
	}

	/*
	 *	Process the delay relative to the start of packet processing
	 */
	if (inst->relative) {
		fr_timeval_add(&when, &request->packet->timestamp, &delay);
	} else {
		fr_timeval_add(&when, now, &delay);
	}

	/*
	 *	If when is in the past (and reschedule is not forced), just return noop
	 */
	cmp = fr_timeval_cmp(now, &when);
	if (!inst->force_reschedule && (cmp >= 0)) return RLM_MODULE_NOOP;

	if (cmp < 0) {
		struct timeval actual;
		fr_timeval_subtract(&actual, &when, now);

		RDEBUG2("Delaying request by ~%"PRIu64".%06"PRIu64"s",
			(uint64_t)actual.tv_sec, (uint64_t)actual.tv_usec);
	} else {
		RDEBUG2("Rescheduling request");
	}

	if (unlang_event_timeout_add(request, delay_done, inst, now, &when) < 0) return RLM_MODULE_FAIL;

	return RLM_MODULE_YIELD;
}

static rlm_rcode_t CC_HINT(nonnull) mod_delay(void *instance, REQUEST *request)
{
	rlm_delay_t	*inst = instance;
	rlm_rcode_t	rcode;

	rcode = delay_add(inst, request);
	if (rcode != RLM_MODULE_YIELD) return rcode;

	return unlang_yield(request, delay_return, NULL, NULL);
}

extern rad_module_t rlm_delay;
rad_module_t rlm_delay = {
	.magic		= RLM_MODULE_INIT,
	.name		= "delay",
	.type		= 0,
	.inst_size	= sizeof(rlm_delay_t),
	.config		= module_config,
	.methods = {
		[MOD_PREACCT]		= mod_delay,
		[MOD_AUTHORIZE]		= mod_delay,
		[MOD_POST_AUTH]		= mod_delay,
	},
};
