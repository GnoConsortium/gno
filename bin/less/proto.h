/* brac.c */
public void match_brac(int obrac, int cbrac, int forwdir, int n);
/* ch.c */
public void end_logfile(void);
public void synch_logfile(void);
public int ch_seek(register POSITION pos);
public int ch_end_seek(void);
public int ch_beg_seek(void);
public POSITION ch_length(void);
public POSITION ch_tell(void);
public int ch_forw_get(void);
public int ch_back_get(void);
public int ch_nbuf(int want_nbufs);
public void ch_flush(void);
public void ch_pipe(void);
public void ch_nonpipe(void);
/* from charset.c */
public void init_charset(void);
public int binary_char(int c);
public int control_char(int c);
public char *prchar(int c);
/* from cmdbuf.c */
public void cmd_reset(void);
public int len_cmdbuf(void);
public int cmd_erase(void);
public int cmd_char(int c);
public int cmd_int(void);
public void cmd_putstr(char *s);
public char *get_cmdbuf(void);
/* command.c */
public void ungetcc(int c);
public void ungetsc(char *s);
public void commands(void);
/* decode.c */
public int cmd_decode(char *cmd, char **sp);
public int add_cmdtable(char *filename);
public void add_hometable(void);
/* edit.c */
public int edit(register char *filename, int just_looking);
public void edit_list(char *list);
public int edit_first(void);
public int edit_last(void);
public int edit_next(int n);
public int edit_prev(int n);
public int edit_index(int n);
public void cat_file(void);
public void use_logfile(char *filename);
/* filename.c */
public char *homefile(char *filename);
public char *find_helpfile(void);
public char *fexpand(char *s);
int bin_file(int f);
public char *glob(char *filename);
public char *load_file(char *filename);
public POSITION filesize(int f);
public char *bad_file(char *filename);
public POSITION filesize(int f);
/* forwback.c */
public void forw(register int n, POSITION pos, int force, int only_last,
                 int nblank);
public void back(register int n, POSITION pos, int force, int only_last);
public void forward(register int n, int force, int only_last);
public void backward(register int n, int force, int only_last);
public int get_back_scroll(void);
/* help.c */
public void help(void);
/* ifile.c */
public IFILE next_ifile(IFILE h);
public IFILE prev_ifile(IFILE h);
public int nifile(void);
public IFILE get_ifile(char *filename, IFILE prev);
public char *get_filename(IFILE ifile);
public int get_index(IFILE ifile);
public void store_pos(IFILE ifile, struct scrpos *scrpos);
public void get_pos(IFILE ifile, struct scrpos *scrpos);

/* from input.c */
public POSITION forw_line(POSITION curr_pos);
public POSITION back_line(POSITION curr_pos);

/* jump.c */
public void jump_forw(void);
public void jump_back(int n);
public void repaint(void);
public void jump_repaint(int percent);
public void jump_line_loc(POSITION pos, int sline);
public void jump_loc(POSITION pos, int sline);
/* line.c */
void prewind(void);
public void plinenum(POSITION pos);
int attr_swidth(int a);
int attr_ewidth(int a);
public int pappend(register int c);
public void pdone(int endline);
public int gline(register int i, register int *ap);
public void null_line(void);
public POSITION forw_raw_line(POSITION curr_pos, char **linep);
public POSITION back_raw_line(POSITION curr_pos, char **linep);
/* linenum.c */
public void clr_linenum(void);
public void add_lnum(int lno, POSITION pos);
public int find_linenum(POSITION pos);
public POSITION find_pos(int lno);
public int currline(int where);
/* lsystem.c */
public void lsystem(char *cmd);
public int pipe_mark(int c, char *cmd);
public int pipe_data(char *cmd, POSITION spos, POSITION epos);
/* main.c */
public void strtcpy(char *to, char *from, unsigned int len);
public char *save(char *s);
public VOID_POINTER ecalloc(int count, unsigned int size);
public char *skipsp(register char *s);
public void quit(int status);
/* mark.c */
public void init_mark(void);
public int badmark(int c);
public void setmark(int c);
public void lastmark(void);
public void gomark(int c);
public POSITION markpos(int c);
/* optfunc */
public void opt_o(int type, char *s);
public void opt__O(int type, char *s);
public void opt_l(int type, char *s);
public void opt__L(int type, char *s);
public void opt_k(int type, char *s);
public void opt_t(int type, char *s);
public void opt__T(int type, char *s);
public void opt_p(int type, register char *s);
public void opt__P(int type, register char *s);
public void opt_b(int type, char *s);
public void opt_v(int type, register char *s);
public void opt_W(int type, register char *s);
public void opt_query(int type, char *s);

/* from option.c */

public void scan_option(char *s);
public void toggle_option(int c, char *s, int how_toggle);
public int single_char_option(int c);
public char *opt_prompt(int c);
public int isoptpending(void);
public void nopendopt(void);
public int getnum(char **sp, int c, int *errp);

/* from opttbl.c */
public void init_option(void);
public struct option *findopt(int c);

/* from os.c */

public int iread(int fd, char *buf, unsigned int len);
public void intread(void);
public long get_time(void);
public char *errno_message(char *filename);

/* from output.c */

public void put_line(void);
public void flush(void);
public void putchr(int c);
public void putstr(char *s);
public void error(char *fmt, PARG *parg);
public void ierror(char *fmt, PARG *parg);
public int query(char *fmt, PARG *parg);

/* from position.c */

public POSITION position(int where);
public void add_forw_pos(POSITION pos);
public void add_back_pos(POSITION pos);
public void pos_clear(void);
public void pos_init(void);
public int onscreen(POSITION pos);
public int empty_screen(void);
public int empty_lines(int s, int e);
public void get_scrpos(struct scrpos *scrpos);
public int adjsline(int sline);

/* from prompt.c */

public void init_prompt(void);
public char *pr_expand(char *proto, int maxwidth);
public char *eq_message(void);
public char *pr_string(void);

/* from screen.c */

public void raw_mode(int on);
public void scrsize(int *p_height, int *p_width);
public void get_term(void);
public void init(void);
public void deinit(void);
public void home(void);
public void add_line(void);
public void lower_left(void);
public void bell(void);
public void vbell(void);
public void clear(void);
public void clear_eol(void);
public void so_enter(void);
public void so_exit(void);
public void ul_enter(void);
public void ul_exit(void);
public void bo_enter(void);
public void bo_exit(void);
public void bl_enter(void);
public void bl_exit(void);
public void backspace(void);
public void putbs(void);

/* from search.c */

public int search(int search_type, char *pattern, int n);

/* from signal.c */

public void fake_interrupt(void);
public HANDLER winch(int type);
public void init_signals(int on);
public void psignals(void);

/* from tags.c */

public void findtag(char *tag);
public int tagsearch(void);

/* from ttyin.c */

public void open_getchr(void);
public int getchr(void);
