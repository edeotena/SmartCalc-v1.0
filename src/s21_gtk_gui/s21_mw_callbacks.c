#include <math.h>

#include "s21_gtk_gui.h"
#include "s21_smart_calc.h"

int check_for_x(queue *tokens) {
  int res = FAILURE;

  while (res == FAILURE && tokens != NULL) {
    if (tokens->op == X) {
      res = SUCCESS;
    }
    tokens = tokens->next;
  }

  return res;
}

int get_double_from_entry(GtkWidget *entry, double *res) {
  int valid = SUCCESS;
  char *val_str = (char *)gtk_entry_get_text(GTK_ENTRY(entry));
  if (*val_str != '\0') {
    char *end;
    *res = strtod(val_str, &end);
    if (*end != '\0') {
      valid = FAILURE;
    }
  } else {
    valid = FAILURE;
  }

  return valid;
}

int get_int_from_entry(GtkWidget *entry, long *res) {
  int valid = SUCCESS;
  char *val_str = (char *)gtk_entry_get_text(GTK_ENTRY(entry));
  if (*val_str != '\0') {
    char *end;
    *res = strtol(val_str, &end, 10);
    if (*end != '\0') {
      valid = FAILURE;
    }
  } else {
    valid = FAILURE;
  }

  return valid;
}

void do_calculate(GtkWidget *window, struct widgets_container *data) {
  if (window == NULL) {
    return;
  }
  char buffer[255] = "Strange error\n";

  double x = 0;
  int valid_x = get_double_from_entry(data->x_field, &x);

  queue *parsed;
  int code = parse_to_tokens(
      (char *)gtk_entry_get_text(GTK_ENTRY(data->calc_field)), &parsed);
  if (check_for_x(parsed) == SUCCESS && valid_x == FAILURE && code == SUCCESS) {
    sprintf(buffer, "Invalid X\n");
    code = FAILURE;
  } else if (code == FAILURE) {
    sprintf(buffer, "Parser error\n");
  } else {
    queue *rpn = make_rpn(parsed, &code);
    if (code == FAILURE) {
      sprintf(buffer, "RPN making error\n");
    } else {
      double res = calc_rpn(rpn, x, &code);
      if (code == FAILURE) {
        sprintf(buffer, "RPN calculation error\n");
      } else {
        if (isnan(res)) {
          sprintf(buffer, "result: NOT A NUMBER(probably root of negative)\n");
        } else if (isinf(res)) {
          sprintf(buffer, "result: INFINITY(probably zero-division)\n");
        } else {
          sprintf(buffer, "result: %.7lf\n", res);
        }
      }
      free_queue(&rpn);
    }
    free_queue(&parsed);
  }

  gtk_label_set_text(GTK_LABEL(data->result), buffer);
}

void do_build(GtkWidget *window, struct widgets_container *data) {
  if (window == NULL) {
    return;
  }

  long steps = 0;
  int valid_steps = get_int_from_entry(data->steps_field, &steps);
  double start = 0, end = 0;
  int valid_start = get_double_from_entry(data->st_field, &start);
  int valid_end = get_double_from_entry(data->end_field, &end);
  double ystart = 0, yend = 0;
  int yvalid_start = get_double_from_entry(data->yst_field, &ystart);
  int yvalid_end = get_double_from_entry(data->yend_field, &yend);

  int code = SUCCESS;

  if (valid_steps == FAILURE || valid_start == FAILURE ||
      yvalid_end == FAILURE || yvalid_start == FAILURE ||
      valid_end == FAILURE || end <= start || steps < 2) {
    code = FAILURE;
  }

  if (code == SUCCESS) {
    queue *parsed;
    code = parse_to_tokens(
        (char *)gtk_entry_get_text(GTK_ENTRY(data->calc_field)), &parsed);
    if (code == SUCCESS) {
      queue *rpn = make_rpn(parsed, &code);
      if (code == SUCCESS) {
        free_queue(&rpn);
      }
      free_queue(&parsed);
    }
  }

  char *line = (char *)gtk_entry_get_text(GTK_ENTRY(data->calc_field));
  char gnu_str[400] = "plot ";
  for (int i = 0, skip = 5; i < 255; ++i) {
    gnu_str[i + skip] = line[i];
    if (line[i] == '^') {
      gnu_str[i + skip] = '*';
      ++skip;
      gnu_str[i + skip] = '*';
    }
  }

  if (code == SUCCESS) {
    char *commands_gnu_plot[] = {"set terminal png enhanced truecolor",
                                 "set output \"tempfile.png\"", gnu_str};
    FILE *gnu_plot = popen("gnuplot -persistent", "w");
    int i;

    for (i = 0; i < 3; i++) {
      fprintf(gnu_plot, "%s \n", commands_gnu_plot[i]);
    }
  }

  if (code == FAILURE) {
    gtk_label_set_text(GTK_LABEL(data->result), "Error\n");
  }
}