/* ftdump -- Dump raw event data from a Feather-Trace event stream. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "mapping.h"

#include "timestamp.h"

static void dump(struct timestamp* ts, size_t count)
{
	struct timestamp *x;
	uint32_t last_seq = 0, next_seq;
	const char* name;
	while (count--) {
		x = ts++;
		name = event2str(x->event);
		next_seq = last_seq + 1;
		if (last_seq && next_seq != x->seq_no)
			printf("==== non-consecutive sequence number ====\n");
		last_seq = x->seq_no;
		if (name)
			printf("%-20s seq:%u  pid:%u  timestamp:%llu  cpu:%d"
			       "  type:%-8s irq:%u irqc:%02u \n",
			       name,  x->seq_no,
			       x->pid,
			       (unsigned long long)  x->timestamp,
			       x->cpu,
			       task_type2str(x->task_type),
			       x->irq_flag,
			       x->irq_count);
		else
			printf("%16s:%3u seq:%u pid:%u  timestamp:%llu  cpu:%u"
			       "  type:%-8s irq:%u irqc:%02u\n",
			       "event",
			       (unsigned int) x->event, x->seq_no, x->pid,
			       (unsigned long long)  x->timestamp,
			       x->cpu,
			       task_type2str(x->task_type),
			       x->irq_flag,
			       x->irq_count);
	}
}

static void die(char* msg)
{
	if (errno)
		perror("error: ");
	fprintf(stderr, "%s\n", msg);
	exit(1);
}

#define offset(type, member)  ((unsigned long) &((type *) 0)->member)

int main(int argc, char** argv)
{
	void* mapped;
	size_t size, count;
	struct timestamp* ts;

	printf("struct timestamp:\n"
	       "\t size              = %3lu\n"
	       "\t offset(seq_no)    = %3lu\n"
	       "\t offset(cpu)       = %3lu\n"
	       "\t offset(event)     = %3lu\n",
	       (unsigned long) sizeof(struct timestamp),
	       offset(struct timestamp, seq_no),
	       offset(struct timestamp, cpu),
	       offset(struct timestamp, event));

	if (argc != 2)
		die("Usage: ftdump  <logfile>");
	if (map_file(argv[1], &mapped, &size))
		die("could not map file");

	ts    = (struct timestamp*) mapped;
	count = size / sizeof(struct timestamp);

	dump(ts, count);
	return 0;
}
