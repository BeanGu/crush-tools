/********************************
   Copyright 2008, 2009 Google Inc.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
 ********************************/
#include <crush/dbfr.h>
#include <crush/general.h>

#include "aggregate_main.h"
#include "aggregate.h"

#define AGG_TMP_BUF_SIZE 64

char *delim;
struct agg_conf conf;

int configure_aggregation(struct agg_conf *conf, struct cmdargs *args,
                          const char *header, const char *delim) {
  if (args->keys) {
    conf->keys.count = expand_nums(args->keys, &(conf->keys.indexes),
                                   &(conf->keys.size));
  } else if (args->key_labels) {
    conf->keys.count = expand_label_list(args->key_labels, header,
                                         delim, &(conf->keys.indexes),
                                         &(conf->keys.size));
    args->preserve = 1;
  }
  if (conf->keys.count < 0)
    return conf->keys.count;
  decrement_values(conf->keys.indexes, conf->keys.count);

  if (args->sums) {
    conf->sums.count = expand_nums(args->sums, &(conf->sums.indexes),
                                   &(conf->sums.size));
  } else if (args->sum_labels) {
    conf->sums.count = expand_label_list(args->sum_labels, header,
                                         delim, &(conf->sums.indexes),
                                         &(conf->sums.size));
    args->preserve = 1;
  }
  if (conf->sums.count < 0) {
    return conf->sums.count;
  } else if (conf->sums.count > 0) {
    decrement_values(conf->sums.indexes, conf->sums.count);
    conf->sums.precisions = xcalloc(conf->sums.count, sizeof(int));
  }

  if (args->counts) {
    conf->counts.count = expand_nums(args->counts, &(conf->counts.indexes),
                                &(conf->counts.size));
  } else if (args->count_labels) {
    conf->counts.count = expand_label_list(args->count_labels, header,
                                      delim, &(conf->counts.indexes),
                                      &(conf->counts.size));
    args->preserve = 1;
  }
  if (conf->counts.count < 0)
    return conf->counts.count;
  else if (conf->counts.count > 0)
    decrement_values(conf->counts.indexes, conf->counts.count);

  if (args->averages) {
    conf->averages.count = expand_nums(args->averages, &(conf->averages.indexes),
                                  &(conf->averages.size));
  } else if (args->average_labels) {
    conf->averages.count = expand_label_list(args->average_labels, header,
                                        delim, &(conf->averages.indexes),
                                        &(conf->averages.size));
    args->preserve = 1;
  }
  if (conf->averages.count < 0) {
    return conf->averages.count;
  } else if (conf->averages.count > 0) {
    decrement_values(conf->averages.indexes, conf->averages.count);
    conf->averages.precisions = xcalloc(conf->averages.count, sizeof(int));
  }

  if (args->mins) {
    conf->mins.count = expand_nums(args->mins, &(conf->mins.indexes),
                              &(conf->mins.size));
  } else if (args->min_labels) {
    conf->mins.count = expand_label_list(args->min_labels, header, delim,
                                    &(conf->mins.indexes),
                                    &(conf->mins.size));
    args->preserve = 1;
  }
  if (conf->mins.count < 0) {
    return conf->mins.count;
  } else if (conf->mins.count > 0) {
    decrement_values(conf->mins.indexes, conf->mins.count);
    conf->mins.precisions = xcalloc(conf->mins.count, sizeof(int));
  }

  if (args->maxs) {
    conf->maxs.count = expand_nums(args->maxs, &(conf->maxs.indexes),
                              &(conf->maxs.size));
  } else if (args->max_labels) {
    conf->maxs.count = expand_label_list(args->max_labels, header, delim,
                                    &(conf->maxs.indexes),
                                    &(conf->maxs.size));
    args->preserve = 1;
  }
  if (conf->maxs.count < 0) {
    return conf->maxs.count;
  } else if (conf->maxs.count > 0) {
    decrement_values(conf->maxs.indexes, conf->maxs.count);
    conf->maxs.precisions = xcalloc(conf->maxs.count, sizeof(int));
  }

  return 0;
}

/** @brief
  *
  * @param args contains the parsed cmd-line options & arguments.
  * @param argc number of cmd-line arguments.
  * @param argv list of cmd-line arguments
  * @param optind index of the first non-option cmd-line argument.
  *
  * @return exit status for main() to return.
  */
int aggregate(struct cmdargs *args, int argc, char *argv[], int optind) {

  int i, n;

  hashtbl_t aggregations;
  struct aggregation *value = NULL;
  char **key_array;

  size_t n_hash_elems;

  FILE *in;                     /* input file */
  dbfr_t *in_reader;

  char *outbuf;                 /* buffer for a line of output */
  size_t outbuf_sz;             /* size of the output buffer */

  char default_delim[] = { 0xFE, 0x00 };  /* default delimiter string */

  delim = args->delim;
  if (!delim)
    delim = getenv("DELIMITER");

  if (delim)
    expand_chars(delim);
  else
    delim = default_delim;

  if (optind == argc)
    in = stdin;
  else
    in = nextfile(argc, argv, &optind, "r");

  if (in == NULL)
    return EXIT_FILE_ERR;

  in_reader = dbfr_init(in);

  memset(&conf, 0, sizeof(conf));
  if (configure_aggregation(&conf, args, in_reader->next_line, delim) != 0) {
    fprintf(stderr, "%s: error parsing field arguments.\n", argv[0]);
    return EXIT_HELP;
  }

#ifdef CRUSH_DEBUG
  fprintf(stderr, "%d keys: ", conf.keys.count);
  for (i = 0; i < conf.keys.count; i++)
    fprintf(stderr, "%d ", conf.keys.indexes[i]);
  fprintf(stderr, "\n%d sums: ", conf.sums.count);
  for (i = 0; i < conf.sums.count; i++)
    fprintf(stderr, "%d ", conf.sums.indexes[i]);
  fprintf(stderr, "\n%d averages: ", conf.averages.count);
  for (i = 0; i < conf.averages.count; i++)
    fprintf(stderr, "%d ", conf.averages.indexes[i]);
  fprintf(stderr, "\n%d counts: ", conf.counts.count);
  for (i = 0; i < conf.counts.count; i++)
    fprintf(stderr, "%d ", conf.counts.indexes[i]);
  fprintf(stderr, "\n\n");
#endif

  outbuf = xmalloc(64);
  outbuf_sz = 64;

  /* set locale with values from the environment so strcoll()
     will work correctly. */
  setlocale(LC_ALL, "");
  setlocale(LC_COLLATE, "");

  if (args->preserve) {
    if (dbfr_getline(in_reader) <= 0) {
      fprintf(stderr, "%s: unexpected end of file\n", getenv("_"));
      exit(EXIT_FILE_ERR);
    }
    chomp(in_reader->current_line);

    if (in_reader->current_line_len > outbuf_sz) {
      outbuf = xrealloc(outbuf, in_reader->current_line_len + 32);
      outbuf_sz = in_reader->current_line_len + 32;
    }

    n = 0; // count output columns
    if (conf.keys.count) {
      extract_fields_to_string(in_reader->current_line, outbuf, outbuf_sz,
                               conf.keys.indexes, conf.keys.count, delim, NULL);
      fputs(outbuf, stdout);
      n++;
    }
    if (args->labels) {
      printf("%s%s", (n++ > 0 ? delim : ""), args->labels);
    } else {
      if (conf.sums.count) {
        extract_fields_to_string(in_reader->current_line, outbuf, outbuf_sz,
                                 conf.sums.indexes, conf.sums.count, delim,
                                 args->auto_label ? "-Sum" : NULL);
        printf("%s%s", (n++ > 0 ? delim : ""), outbuf);
      }

      if (conf.counts.count) {
        extract_fields_to_string(in_reader->current_line, outbuf, outbuf_sz,
                                 conf.counts.indexes, conf.counts.count, delim,
                                 args->auto_label ? "-Count" : NULL);
        printf("%s%s", (n++ > 0 ? delim : ""), outbuf);
      }

      if (conf.averages.count) {
        extract_fields_to_string(in_reader->current_line, outbuf, outbuf_sz,
                                 conf.averages.indexes, conf.averages.count,
                                 delim, args->auto_label ? "-Average" : NULL);
        printf("%s%s", (n++ > 0 ? delim : ""), outbuf);
      }

      if (conf.mins.count) {
        extract_fields_to_string(in_reader->current_line, outbuf, outbuf_sz,
                                 conf.mins.indexes, conf.mins.count, delim,
                                 args->auto_label ? "-Min" : NULL);
        printf("%s%s", (n++ > 0 ? delim : ""), outbuf);
      }

      if (conf.maxs.count) {
        extract_fields_to_string(in_reader->current_line, outbuf, outbuf_sz,
                                 conf.maxs.indexes, conf.maxs.count, delim,
                                 args->auto_label ? "-Max" : NULL);
        printf("%s%s", (n++ > 0 ? delim : ""), outbuf);
      }
    }

    fputs("\n", stdout);
  }

  ht_init(&aggregations, 1024, NULL, (void (*)) free_agg);

  n_hash_elems = 0;
  memset(outbuf, 0, outbuf_sz);

  /* loop through all files */
  while (in != NULL) {
    char tmpbuf[AGG_TMP_BUF_SIZE];
    size_t tmplen;
    int in_hash;

    /* loop through each line of the file */
    while (dbfr_getline(in_reader) > 0) {
      chomp(in_reader->current_line);
      if (conf.keys.count) {
        if (in_reader->current_line_len > outbuf_sz) {
          outbuf = xrealloc(outbuf, in_reader->current_line_len + 32);
          outbuf_sz = in_reader->current_line_len + 32;
        }
        extract_fields_to_string(in_reader->current_line, outbuf, outbuf_sz,
                                 conf.keys.indexes, conf.keys.count, delim, NULL);

        value = (struct aggregation *) ht_get(&aggregations, outbuf);
      }

      if (!value) {
        in_hash = 0;
        value = alloc_agg(conf.sums.count, conf.counts.count,
                          conf.averages.count, conf.mins.count,
                          conf.maxs.count);
      } else {
        in_hash = 1;
      }

      /* sums */
      for (i = 0; i < conf.sums.count; i++) {
        tmplen =
          get_line_field(tmpbuf, in_reader->current_line,
                         AGG_TMP_BUF_SIZE - 1, conf.sums.indexes[i], delim);
        if (tmplen > 0) {
          n = float_str_precision(tmpbuf);
          if (conf.sums.precisions[i] < n)
            conf.sums.precisions[i] = n;
          value->sums[i] += atof(tmpbuf);
        }
      }

      /* averages */
      for (i = 0; i < conf.averages.count; i++) {
        tmplen = get_line_field(tmpbuf, in_reader->current_line,
                                AGG_TMP_BUF_SIZE - 1, conf.averages.indexes[i],
                                delim);
        if (tmplen > 0) {
          n = float_str_precision(tmpbuf);
          if (conf.averages.precisions[i] < n)
            conf.averages.precisions[i] = n;
          value->average_sums[i] += atof(tmpbuf);
          value->average_counts[i] += 1;
        }
      }

      /* counts */
      for (i = 0; i < conf.counts.count; i++) {
        tmplen = get_line_field(tmpbuf, in_reader->current_line,
                                AGG_TMP_BUF_SIZE - 1, conf.counts.indexes[i],
                                delim);
        if (tmplen > 0) {
          value->counts[i] += 1;
        }
      }

      /* mins */
      for (i = 0; i < conf.mins.count; i++) {
        tmplen = get_line_field(tmpbuf, in_reader->current_line,
                                AGG_TMP_BUF_SIZE - 1, conf.mins.indexes[i],
                                delim);
        if (tmplen > 0) {
          double cur_val;
          n = sscanf(tmpbuf, "%lf", &cur_val);
          if (n) {
            if (cur_val < value->numeric_mins[i] ||
                ! value->mins_initialized[i]) {
              value->numeric_mins[i] = cur_val;
              conf.mins.precisions[i] = float_str_precision(tmpbuf);
            }
            value->mins_initialized[i] = 1;
          }
        }
      }

      /* maxs */
      for (i = 0; i < conf.maxs.count; i++) {
        tmplen = get_line_field(tmpbuf, in_reader->current_line,
                                AGG_TMP_BUF_SIZE - 1, conf.maxs.indexes[i],
                                delim);
        if (tmplen > 0) {
          double cur_val;
          n = sscanf(tmpbuf, "%lf", &cur_val);
          if (n) {
            if (cur_val > value->numeric_maxs[i] ||
                ! value->maxs_initialized[i]) {
              value->numeric_maxs[i] = cur_val;
              conf.maxs.precisions[i] = float_str_precision(tmpbuf);
            }
            value->maxs_initialized[i] = 1;
          }
        }
      }

      if (!in_hash) {
        if (ht_put(&aggregations, outbuf, value) != 0)
          fprintf(stderr, "%s: failed to store value in hashtable.\n",
                  getenv("_"));
        n_hash_elems++;
      }

    }
    dbfr_close(in_reader);
    in = nextfile(argc, argv, &optind, "r");
    if (in) {
      in_reader = dbfr_init(in);
      /* reconfigure fields (needed if labels were used) */
      if (configure_aggregation(&conf, args, in_reader->next_line, delim) != 0) {
        fprintf(stderr, "%s: error parsing field arguments.\n", argv[0]);
        return EXIT_HELP;
      }
      if (args->preserve)
        dbfr_getline(in_reader);
    }
  }

  free(outbuf);

  /* Print all of the output. */
  if (conf.keys.count) {
    key_array = xmalloc(sizeof(char *) * n_hash_elems);
    ht_keys(&aggregations, key_array);
    if (! args->nosort) {
      qsort(key_array, n_hash_elems, sizeof(char *),
            (int (*)(const void *, const void *)) key_strcmp);
    }
    for (i = 0; i < n_hash_elems; i++) {
      value = (struct aggregation *) ht_get(&aggregations, key_array[i]);
      print_keys_and_agg_vals(key_array[i], value);
    }
    free(key_array);
  } else {
    print_keys_and_agg_vals(NULL, value);
  }

  ht_destroy(&aggregations);

  return EXIT_OKAY;
}

int key_strcmp(char **a, char **b) {
  char fa[256], fb[256];
  int retval = 0;
  int i;
  int alen, blen;

  /* avoid comparing nulls */
  if (!*a && !*b)
    return 0;
  if (!*a && *b)
    return -1;
  if (*a && !*b)
    return 1;

  i = 0;
  while (retval == 0) {
    alen = get_line_field(fa, *a, 255, i, delim);
    blen = get_line_field(fb, *b, 255, i, delim);
    if (alen < 0 || blen < 0)
      break;
    retval = strcoll(fa, fb);
    i++;
  }

  return retval;
}

int float_str_precision(char *d) {
  char *p;
  int after_dot;
  if (!d)
    return 0;

  p = strchr(d, '.');
  if (p == NULL) {
    return 0;
  }
  after_dot = p - d + 1;
  return (strlen(d) - after_dot);
}

int print_keys_and_agg_vals(char *key, struct aggregation *val) {
  int i, n = 0;
  if (key) {
    fputs(key, stdout);
    n++;
  }
  for (i = 0; i < conf.sums.count; i++) {
    printf("%s%.*f", (n++ > 0 ? delim : ""), conf.sums.precisions[i], val->sums[i]);
  }
  for (i = 0; i < conf.counts.count; i++) {
    printf("%s%d", (n++ > 0 ? delim : ""), val->counts[i]);
  }
  for (i = 0; i < conf.averages.count; i++) {
    printf("%s%.*f", (n++ > 0 ? delim : ""), conf.averages.precisions[i] + 2,
           val->average_sums[i] / val->average_counts[i]);
  }
  for (i = 0; i < conf.mins.count; i++) {
    if (val->mins_initialized[i])
      printf("%s%.*f", (n++ > 0 ? delim : ""), conf.mins.precisions[i], val->numeric_mins[i]);
    else if (n++ > 0)
      fputs(delim, stdout);
  }
  for (i = 0; i < conf.maxs.count; i++) {
    if (val->maxs_initialized[i])
      printf("%s%.*f", (n++ > 0 ? delim : ""), conf.maxs.precisions[i], val->numeric_maxs[i]);
    else if (n++ > 0)
      fputs(delim, stdout);
  }
  fputs("\n", stdout);
  return 0;
}

void ht_print_keys_and_agg_vals(void *htelem) {
  char *key;
  struct aggregation *val;
  key = ((ht_elem_t *) htelem)->key;
  val = ((ht_elem_t *) htelem)->data;
  print_keys_and_agg_vals(key, val);
}

void extract_fields_to_string(char *line, char *destbuf, size_t destbuf_sz,
                              int *fields, size_t nfields, char *delim,
                              char *suffix) {
  char *pos;
  int i, field_len = 0;
  size_t delim_len = 0, suffix_len = 0;

  delim_len = strlen(delim);
  if (suffix)
    suffix_len = strlen(suffix);
  pos = destbuf;

  for (i = 0; i < nfields; i++) {
    if ((field_len = get_line_field(pos, line, destbuf_sz - (pos - destbuf),
                                    fields[i], delim)) < 0)
      DIE("Cant find field %d.\n", fields[i]);

    pos += field_len;
    if (suffix) {
    	strncat(pos, suffix, destbuf_sz - (pos - destbuf));
    	pos += suffix_len;
    }
    if (i != nfields - 1) {
    	strncat(pos, delim, destbuf_sz - (pos - destbuf));
      pos += delim_len;
    }
  }
}

void decrement_values(int *array, size_t sz) {
  int j;
  if (array == NULL || sz == 0)
    return;
  for (j = 0; j < sz; j++) {
    array[j]--;
  }
}

struct aggregation *alloc_agg(int nsum, int ncount, int naverage, int nmin,
                              int nmax) {
  struct aggregation *agg;

  agg = xmalloc(sizeof(struct aggregation));
  memset(agg, 0, sizeof(struct aggregation));

  if (nsum > 0) {
    agg->sums = xmalloc(sizeof(double) * nsum);
    memset(agg->sums, 0, sizeof(double) * nsum);
  }

  if (ncount > 0) {
    agg->counts = xmalloc(sizeof(u_int32_t) * ncount);
    memset(agg->counts, 0, sizeof(u_int32_t) * ncount);
  }

  if (naverage > 0) {
    agg->average_sums = xmalloc(sizeof(double) * naverage);
    memset(agg->average_sums, 0, sizeof(double) * naverage);

    agg->average_counts = xmalloc(sizeof(u_int32_t) * naverage);
    memset(agg->average_counts, 0, sizeof(u_int32_t) * naverage);
  }

  if (nmin > 0) {
    agg->numeric_mins = xmalloc(sizeof(double) * nmin);
    memset(agg->numeric_mins, 0, sizeof(double) * nmin);
    agg->mins_initialized = xmalloc(sizeof(int) * nmin);
    memset(agg->mins_initialized, 0, nmin);
  }

  if (nmax > 0) {
    agg->numeric_maxs = xmalloc(sizeof(double) * nmax);
    memset(agg->numeric_maxs, 0, sizeof(double) * nmax);
    agg->maxs_initialized = xmalloc(sizeof(int) * nmax);
    memset(agg->maxs_initialized, 0, nmax);
  }
  return agg;
}

void free_agg(struct aggregation *agg) {
  if (!agg)
    return;
  if (agg->counts)
    free(agg->counts);
  if (agg->sums)
    free(agg->sums);
  if (agg->average_sums)
    free(agg->average_sums);
  if (agg->average_counts)
    free(agg->average_counts);
  if (agg->numeric_mins)
    free(agg->numeric_mins);
  if (agg->numeric_maxs)
    free(agg->numeric_maxs);
  free(agg);
}
