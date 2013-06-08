/** @module
 *
 * @author Jesus Manzanares 
 * @author-email j - . - manzanaresartolazabal - @ - gmail - . - com (remove ' - ')
 *
 * @brief This module constraints wether a teacher can teach a certain event
 * by looking at its subject constraints
 *
 * @ingroup School scheduling, Multiweek scheduling, Subject scheduling
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>

#include "module.h"

struct teacher_subjects {
	int teacher;
	int subject;
};

static int teacher_num;
static struct teacher_subjects *teach_subs;

//static int periods, days;

int getsubjects( char *restriction, char *cont, resource *res1)
{
	resource *subject;
	resourcetype *subject_type;

	subject_type = restype_find("subject");
	subject = res_find(subject_type, cont);

	if (subject == NULL) {
		error("subject %s not found", cont);
		return(-1);
	}

	//annotate subject for teacher
	teach_subs[teacher_num].teacher = res1->resid;
	teach_subs[teacher_num].subject = subject->resid;
	teacher_num ++;

	return 0;
}

int fitness(chromo **c, ext **e, slist **s)
{
	int sum, n, t;
	chromo *subchrom;
	chromo *teachrom;

	subchrom = c[0];
	teachrom = c[1];

	sum = 0;

	//check all events (tuples)
	for (t=0; t < subchrom->gennum ; t++) {
		//debug("subject has gen %d in pos %d", subchrom->gen[t], t);
		for ( n = 0; n < teacher_num; n++ ) {
			//debug("we are at gen %d, searching for teacher %d", t, n);
			//debug("teacher in chrom is %d", teachrom->gen[t]);
			if (teach_subs[n].teacher == teachrom->gen[t]) {
				//debug("FOUNDDD the TEACHER");
				if (subchrom->gen[t] != teach_subs[n].subject) {
					//debug("punishment!!!");
					sum++;
				} /*else {
					debug("WINNNNNNN");
				}*/
			}
		}
	}

	return sum;
}

int module_precalc(moduleoption *opt)
{
	resourcetype *teacher;
	resourcetype *subject;

	//Check for needed resources
	teacher=restype_find("teacher");
	if(teacher==NULL) {
		error(_("Resource type '%s' not found"), "teacher");
		return -1;
	}
	subject=restype_find("subject");
	if(subject==NULL) {
		error(_("Resource type '%s' not found"), "subject");
		return -1;
	}
	return 0;
}

int module_init(moduleoption *opt)
{
	fitnessfunc *f;
	resourcetype *teacher_type;

	precalc_new(module_precalc);

	teacher_type=restype_find("teacher");
	
	//init structure before calling handler_res_new
	teacher_num = 0;
	teach_subs = malloc(sizeof(*teach_subs) * teacher_type->resnum);

	handler_res_new( "teacher", "teaches-subject", getsubjects );


	f = fitness_new("subject-teacher",
			option_int(opt, "weight"),
			option_int(opt, "mandatory"),
			fitness);
	if (f == NULL) return -1;

	if (fitness_request_chromo(f, "subject")) return -1;
	if (fitness_request_chromo(f, "teacher")) return -1;

	//if (fitness_request_slist(f, "teacher")) return -1;

	return(0);
}
