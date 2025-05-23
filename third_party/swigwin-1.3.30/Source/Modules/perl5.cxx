/* ----------------------------------------------------------------------------
 * See the LICENSE file for information on copyright, usage and redistribution
 * of SWIG, and the README file for authors - http://www.swig.org/release.html.
 *
 * perl5.cxx
 *
 * Perl5 language module for SWIG.
 * ------------------------------------------------------------------------- */

char cvsroot_perl5_cxx[] = "$Header: /cvsroot/swig/SWIG/Source/Modules/perl5.cxx,v 1.64 2006/11/01 23:54:52 wsfulton Exp $";

#include "swigmod.h"
#include "cparse.h"
static int treduce = SWIG_cparse_template_reduce(0);

#include <ctype.h>

static const char *usage = (char *) "\
Perl5 Options (available with -perl5)\n\
     -static         - Omit code related to dynamic loading\n\
     -nopm           - Do not generate the .pm file\n\
     -proxy          - Create proxy classes\n\
     -noproxy        - Don't create proxy classes\n\
     -const          - Wrap constants as constants and not variables (implies -proxy)\n\
     -nocppcast      - Disable C++ casting operators, useful for generating bugs\n\
     -cppcast        - Enable C++ casting operators\n\
     -compat         - Compatibility mode\n\n";

static int compat = 0;

static int no_pmfile = 0;

static int export_all = 0;

/*
 * pmfile
 *   set by the -pm flag, overrides the name of the .pm file
 */
static String *pmfile = 0;

/*
 * module
 *   set by the %module directive, e.g. "Xerces". It will determine
 *   the name of the .pm file, and the dynamic library.
 */
static String *module = 0;

/*
 * fullmodule
 *   the fully namespace qualified name of the module, e.g. "XML::Xerces"
 *   it will be used to set the package namespace in the .pm file, as
 *   well as the name of the initialization methods in the glue library
 */
static String *fullmodule = 0;
/*
 * cmodule
 *   the namespace of the internal glue code, set to the value of
 *   module with a 'c' appended
 */
static String *cmodule = 0;

static String *command_tab = 0;
static String *constant_tab = 0;
static String *variable_tab = 0;

static File *f_runtime = 0;
static File *f_header = 0;
static File *f_wrappers = 0;
static File *f_init = 0;
static File *f_pm = 0;
static String *pm;		/* Package initialization code */
static String *magic;		/* Magic variable wrappers     */

static int is_static = 0;

/* The following variables are used to manage Perl5 classes */

static int blessed = 1;		/* Enable object oriented features */
static int do_constants = 0;	/* Constant wrapping */
static List *classlist = 0;	/* List of classes */
static int have_constructor = 0;
static int have_destructor = 0;
static int have_data_members = 0;
static String *class_name = 0;	/* Name of the class (what Perl thinks it is) */
static String *real_classname = 0;	/* Real name of C/C++ class */
static String *fullclassname = 0;

static String *pcode = 0;	/* Perl code associated with each class */
						  /* static  String   *blessedmembers = 0;     *//* Member data associated with each class */
static int member_func = 0;	/* Set to 1 when wrapping a member function */
static String *func_stubs = 0;	/* Function stubs */
static String *const_stubs = 0;	/* Constant stubs */
static int num_consts = 0;	/* Number of constants */
static String *var_stubs = 0;	/* Variable stubs */
static String *exported = 0;	/* Exported symbols */
static String *pragma_include = 0;
static String *additional_perl_code = 0;	/* Additional Perl code from %perlcode %{ ... %} */
static Hash *operators = 0;
static int have_operators = 0;

class PERL5:public Language {
public:

  PERL5():Language () {
    Clear(argc_template_string);
    Printv(argc_template_string, "items", NIL);
    Clear(argv_template_string);
    Printv(argv_template_string, "ST(%d)", NIL);
  }

  /* Test to see if a type corresponds to something wrapped with a shadow class */
  Node *is_shadow(SwigType *t) {
    Node *n;
    n = classLookup(t);
    /*  Printf(stdout,"'%s' --> '%x'\n", t, n); */
    if (n) {
      if (!Getattr(n, "perl5:proxy")) {
	setclassname(n);
      }
      return Getattr(n, "perl5:proxy");
    }
    return 0;
  }

  /* ------------------------------------------------------------
   * main()
   * ------------------------------------------------------------ */

  virtual void main(int argc, char *argv[]) {
    int i = 1;
    int cppcast = 1;

    SWIG_library_directory("perl5");

    for (i = 1; i < argc; i++) {
      if (argv[i]) {
	if (strcmp(argv[i], "-package") == 0) {
	  Printf(stderr,
		 "*** -package is no longer supported\n*** use the directive '%module A::B::C' in your interface file instead\n*** see the Perl section in the manual for details.\n");
	  SWIG_exit(EXIT_FAILURE);
	} else if (strcmp(argv[i], "-interface") == 0) {
	  Printf(stderr,
		 "*** -interface is no longer supported\n*** use the directive '%module A::B::C' in your interface file instead\n*** see the Perl section in the manual for details.\n");
	  SWIG_exit(EXIT_FAILURE);
	} else if (strcmp(argv[i], "-exportall") == 0) {
	  export_all = 1;
	  Swig_mark_arg(i);
	} else if (strcmp(argv[i], "-static") == 0) {
	  is_static = 1;
	  Swig_mark_arg(i);
	} else if ((strcmp(argv[i], "-shadow") == 0) || ((strcmp(argv[i], "-proxy") == 0))) {
	  blessed = 1;
	  Swig_mark_arg(i);
	} else if ((strcmp(argv[i], "-noproxy") == 0)) {
	  blessed = 0;
	  Swig_mark_arg(i);
	} else if (strcmp(argv[i], "-const") == 0) {
	  do_constants = 1;
	  blessed = 1;
	  Swig_mark_arg(i);
	} else if (strcmp(argv[i], "-nopm") == 0) {
	  no_pmfile = 1;
	  Swig_mark_arg(i);
	} else if (strcmp(argv[i], "-pm") == 0) {
	  Swig_mark_arg(i);
	  i++;
	  pmfile = NewString(argv[i]);
	  Swig_mark_arg(i);
	} else if (strcmp(argv[i], "-cppcast") == 0) {
	  cppcast = 1;
	  Swig_mark_arg(i);
	} else if (strcmp(argv[i], "-nocppcast") == 0) {
	  cppcast = 0;
	  Swig_mark_arg(i);
	} else if (strcmp(argv[i], "-compat") == 0) {
	  compat = 1;
	  Swig_mark_arg(i);
	} else if (strcmp(argv[i], "-help") == 0) {
	  fputs(usage, stdout);
	}
      }
    }

    if (cppcast) {
      Preprocessor_define((DOH *) "SWIG_CPLUSPLUS_CAST", 0);
    }

    Preprocessor_define("SWIGPERL 1", 0);
    Preprocessor_define("SWIGPERL5 1", 0);
    SWIG_typemap_lang("perl5");
    SWIG_config_file("perl5.swg");
    allow_overloading();
  }

  /* ------------------------------------------------------------
   * top()
   * ------------------------------------------------------------ */

  virtual int top(Node *n) {

    /* Initialize all of the output files */
    String *outfile = Getattr(n, "outfile");

    f_runtime = NewFile(outfile, "w");
    if (!f_runtime) {
      FileErrorDisplay(outfile);
      SWIG_exit(EXIT_FAILURE);
    }
    f_init = NewString("");
    f_header = NewString("");
    f_wrappers = NewString("");

    /* Register file targets with the SWIG file handler */
    Swig_register_filebyname("header", f_header);
    Swig_register_filebyname("wrapper", f_wrappers);
    Swig_register_filebyname("runtime", f_runtime);
    Swig_register_filebyname("init", f_init);

    classlist = NewList();

    pm = NewString("");
    func_stubs = NewString("");
    var_stubs = NewString("");
    const_stubs = NewString("");
    exported = NewString("");
    magic = NewString("");
    pragma_include = NewString("");
    additional_perl_code = NewString("");

    command_tab = NewString("static swig_command_info swig_commands[] = {\n");
    constant_tab = NewString("static swig_constant_info swig_constants[] = {\n");
    variable_tab = NewString("static swig_variable_info swig_variables[] = {\n");

    Swig_banner(f_runtime);

    Printf(f_runtime, "#define SWIGPERL\n");
    Printf(f_runtime, "#define SWIG_CASTRANK_MODE\n");


    module = Copy(Getattr(n, "name"));

    /* If we're in blessed mode, change the package name to "packagec" */

    if (blessed) {
      cmodule = NewStringf("%sc", module);
    } else {
      cmodule = NewString(module);
    }
    fullmodule = NewString(module);

    /* Create a .pm file
     * Need to strip off any prefixes that might be found in
     * the module name */

    if (no_pmfile) {
      f_pm = NewString(0);
    } else {
      if (pmfile == NULL) {
	char *m = Char(module) + Len(module);
	while (m != Char(module)) {
	  if (*m == ':') {
	    m++;
	    break;
	  }
	  m--;
	}
	pmfile = NewStringf("%s.pm", m);
      }
      String *filen = NewStringf("%s%s", SWIG_output_directory(), pmfile);
      if ((f_pm = NewFile(filen, "w")) == 0) {
	FileErrorDisplay(filen);
	SWIG_exit(EXIT_FAILURE);
      }
      Delete(filen);
      filen = NULL;
      Swig_register_filebyname("pm", f_pm);
      Swig_register_filebyname("perl", f_pm);
    }
    {
      String *tmp = NewString(fullmodule);
      Replaceall(tmp, ":", "_");
      Printf(f_header, "#define SWIG_init    boot_%s\n\n", tmp);
      Printf(f_header, "#define SWIG_name   \"%s::boot_%s\"\n", cmodule, tmp);
      Printf(f_header, "#define SWIG_prefix \"%s::\"\n", cmodule);
      Delete(tmp);
    }

    Printv(f_pm,
	   "# This file was created automatically by SWIG ", PACKAGE_VERSION, ".\n", "# Don't modify this file, modify the SWIG interface instead.\n", NIL);

    Printf(f_pm, "package %s;\n", fullmodule);

    Printf(f_pm, "require Exporter;\n");
    if (!is_static) {
      Printf(f_pm, "require DynaLoader;\n");
      Printf(f_pm, "@ISA = qw(Exporter DynaLoader);\n");
    } else {
      Printf(f_pm, "@ISA = qw(Exporter);\n");
    }

    /* Start creating magic code */

    Printv(magic,
	   "#ifdef PERL_OBJECT\n",
	   "#define MAGIC_CLASS _wrap_", module, "_var::\n",
	   "class _wrap_", module, "_var : public CPerlObj {\n",
	   "public:\n",
	   "#else\n",
	   "#define MAGIC_CLASS\n",
	   "#endif\n",
	   "SWIGCLASS_STATIC int swig_magic_readonly(pTHX_ SV *SWIGUNUSEDPARM(sv), MAGIC *SWIGUNUSEDPARM(mg)) {\n",
	   tab4, "MAGIC_PPERL\n", tab4, "croak(\"Value is read-only.\");\n", tab4, "return 0;\n", "}\n", NIL);

    Printf(f_wrappers, "#ifdef __cplusplus\nextern \"C\" {\n#endif\n");

    /* emit wrappers */
    Language::top(n);

    String *base = NewString("");

    /* Dump out variable wrappers */

    Printv(magic, "\n\n#ifdef PERL_OBJECT\n", "};\n", "#endif\n", NIL);

    Printf(f_header, "%s\n", magic);

    String *type_table = NewString("");

    /* Patch the type table to reflect the names used by shadow classes */
    if (blessed) {
      Iterator cls;
      for (cls = First(classlist); cls.item; cls = Next(cls)) {
	String *pname = Getattr(cls.item, "perl5:proxy");
	if (pname) {
	  SwigType *type = Getattr(cls.item, "classtypeobj");
	  if (!type)
	    continue;		/* If unnamed class, no type will be found */
	  type = Copy(type);

	  SwigType_add_pointer(type);
	  String *mangled = SwigType_manglestr(type);
	  SwigType_remember_mangleddata(mangled, NewStringf("\"%s\"", pname));
	  Delete(type);
	  Delete(mangled);
	}
      }
    }
    SwigType_emit_type_table(f_runtime, type_table);

    Printf(f_wrappers, "%s", type_table);
    Delete(type_table);

    Printf(constant_tab, "{0,0,0,0,0,0}\n};\n");
    Printv(f_wrappers, constant_tab, NIL);

    Printf(f_wrappers, "#ifdef __cplusplus\n}\n#endif\n");

    Printf(f_init, "\t ST(0) = &PL_sv_yes;\n");
    Printf(f_init, "\t XSRETURN(1);\n");
    Printf(f_init, "}\n");

    /* Finish off tables */
    Printf(variable_tab, "{0,0,0,0}\n};\n");
    Printv(f_wrappers, variable_tab, NIL);

    Printf(command_tab, "{0,0}\n};\n");
    Printv(f_wrappers, command_tab, NIL);


    Printf(f_pm, "package %s;\n", cmodule);

    if (!is_static) {
      Printf(f_pm, "bootstrap %s;\n", fullmodule);
    } else {
      String *tmp = NewString(fullmodule);
      Replaceall(tmp, ":", "_");
      Printf(f_pm, "boot_%s();\n", tmp);
      Delete(tmp);
    }
    Printf(f_pm, "package %s;\n", fullmodule);
    Printf(f_pm, "@EXPORT = qw( %s);\n", exported);
    Printf(f_pm, "%s", pragma_include);

    if (blessed) {

      Printv(base, "\n# ---------- BASE METHODS -------------\n\n", "package ", fullmodule, ";\n\n", NIL);

      /* Write out the TIE method */

      Printv(base, "sub TIEHASH {\n", tab4, "my ($classname,$obj) = @_;\n", tab4, "return bless $obj, $classname;\n", "}\n\n", NIL);

      /* Output a CLEAR method.   This is just a place-holder, but by providing it we
       * can make declarations such as
       *     %$u = ( x => 2, y=>3, z =>4 );
       *
       * Where x,y,z are the members of some C/C++ object. */

      Printf(base, "sub CLEAR { }\n\n");

      /* Output default firstkey/nextkey methods */

      Printf(base, "sub FIRSTKEY { }\n\n");
      Printf(base, "sub NEXTKEY { }\n\n");

      /* Output a FETCH method.  This is actually common to all classes */
      Printv(base,
	     "sub FETCH {\n",
	     tab4, "my ($self,$field) = @_;\n", tab4, "my $member_func = \"swig_${field}_get\";\n", tab4, "$self->$member_func();\n", "}\n\n", NIL);

      /* Output a STORE method.   This is also common to all classes (might move to base class) */

      Printv(base,
	     "sub STORE {\n",
	     tab4, "my ($self,$field,$newval) = @_;\n",
	     tab4, "my $member_func = \"swig_${field}_set\";\n", tab4, "$self->$member_func($newval);\n", "}\n\n", NIL);

      /* Output a 'this' method */

      Printv(base, "sub this {\n", tab4, "my $ptr = shift;\n", tab4, "return tied(%$ptr);\n", "}\n\n", NIL);

      Printf(f_pm, "%s", base);

      /* Emit function stubs for stand-alone functions */

      Printf(f_pm, "\n# ------- FUNCTION WRAPPERS --------\n\n");
      Printf(f_pm, "package %s;\n\n", fullmodule);
      Printf(f_pm, "%s", func_stubs);

      /* Emit package code for different classes */
      Printf(f_pm, "%s", pm);

      if (num_consts > 0) {
	/* Emit constant stubs */
	Printf(f_pm, "\n# ------- CONSTANT STUBS -------\n\n");
	Printf(f_pm, "package %s;\n\n", fullmodule);
	Printf(f_pm, "%s", const_stubs);
      }

      /* Emit variable stubs */

      Printf(f_pm, "\n# ------- VARIABLE STUBS --------\n\n");
      Printf(f_pm, "package %s;\n\n", fullmodule);
      Printf(f_pm, "%s", var_stubs);
    }

    /* Add additional Perl code at the end */
    Printf(f_pm, "%s", additional_perl_code);

    Printf(f_pm, "1;\n");
    Close(f_pm);
    Delete(f_pm);
    Delete(base);

    /* Close all of the files */
    Dump(f_header, f_runtime);
    Dump(f_wrappers, f_runtime);
    Wrapper_pretty_print(f_init, f_runtime);
    Delete(f_header);
    Delete(f_wrappers);
    Delete(f_init);
    Close(f_runtime);
    Delete(f_runtime);
    return SWIG_OK;
  }

  /* ------------------------------------------------------------
   * importDirective(Node *n)
   * ------------------------------------------------------------ */

  virtual int importDirective(Node *n) {
    if (blessed) {
      String *modname = Getattr(n, "module");
      if (modname) {
	Printf(f_pm, "require %s;\n", modname);
      }
    }
    return Language::importDirective(n);
  }

  /* ------------------------------------------------------------
   * functionWrapper()
   * ------------------------------------------------------------ */

  virtual int functionWrapper(Node *n) {
    String *name = Getattr(n, "name");
    String *iname = Getattr(n, "sym:name");
    SwigType *d = Getattr(n, "type");
    ParmList *l = Getattr(n, "parms");
    String *overname = 0;

    Parm *p;
    int i;
    Wrapper *f;
    char source[256], temp[256];
    String *tm;
    String *cleanup, *outarg;
    int num_saved = 0;
    int num_arguments, num_required;
    int varargs = 0;

    if (Getattr(n, "sym:overloaded")) {
      overname = Getattr(n, "sym:overname");
    } else {
      if (!addSymbol(iname, n))
	return SWIG_ERROR;
    }

    f = NewWrapper();
    cleanup = NewString("");
    outarg = NewString("");

    String *wname = Swig_name_wrapper(iname);
    if (overname) {
      Append(wname, overname);
    }
    Setattr(n, "wrap:name", wname);
    Printv(f->def, "XS(", wname, ") {\n", "{\n",	/* scope to destroy C++ objects before croaking */
	   NIL);

    emit_args(d, l, f);
    emit_attach_parmmaps(l, f);
    Setattr(n, "wrap:parms", l);

    num_arguments = emit_num_arguments(l);
    num_required = emit_num_required(l);
    varargs = emit_isvarargs(l);

    Wrapper_add_local(f, "argvi", "int argvi = 0");

    /* Check the number of arguments */
    if (!varargs) {
      Printf(f->code, "    if ((items < %d) || (items > %d)) {\n", num_required, num_arguments);
    } else {
      Printf(f->code, "    if (items < %d) {\n", num_required);
    }
    Printf(f->code, "        SWIG_croak(\"Usage: %s\");\n", usage_func(Char(iname), d, l));
    Printf(f->code, "}\n");

    /* Write code to extract parameters. */
    i = 0;
    for (i = 0, p = l; i < num_arguments; i++) {

      /* Skip ignored arguments */

      while (checkAttribute(p, "tmap:in:numinputs", "0")) {
	p = Getattr(p, "tmap:in:next");
      }

      SwigType *pt = Getattr(p, "type");

      /* Produce string representation of source and target arguments */
      sprintf(source, "ST(%d)", i);
      String *target = Getattr(p, "lname");

      if (i >= num_required) {
	Printf(f->code, "    if (items > %d) {\n", i);
      }
      if ((tm = Getattr(p, "tmap:in"))) {
	Replaceall(tm, "$target", target);
	Replaceall(tm, "$source", source);
	Replaceall(tm, "$input", source);
	Setattr(p, "emit:input", source);	/* Save input location */

	if (Getattr(p, "wrap:disown") || (Getattr(p, "tmap:in:disown"))) {
	  Replaceall(tm, "$disown", "SWIG_POINTER_DISOWN");
	} else {
	  Replaceall(tm, "$disown", "0");
	}

	Printf(f->code, "%s\n", tm);
	p = Getattr(p, "tmap:in:next");
      } else {
	Swig_warning(WARN_TYPEMAP_IN_UNDEF, input_file, line_number, "Unable to use type %s as a function argument.\n", SwigType_str(pt, 0));
	p = nextSibling(p);
      }
      if (i >= num_required) {
	Printf(f->code, "    }\n");
      }
    }

    if (varargs) {
      if (p && (tm = Getattr(p, "tmap:in"))) {
	sprintf(source, "ST(%d)", i);
	Replaceall(tm, "$input", source);
	Setattr(p, "emit:input", source);
	Printf(f->code, "if (items >= %d) {\n", i);
	Printv(f->code, tm, "\n", NIL);
	Printf(f->code, "}\n");
      }
    }

    /* Insert constraint checking code */
    for (p = l; p;) {
      if ((tm = Getattr(p, "tmap:check"))) {
	Replaceall(tm, "$target", Getattr(p, "lname"));
	Printv(f->code, tm, "\n", NIL);
	p = Getattr(p, "tmap:check:next");
      } else {
	p = nextSibling(p);
      }
    }

    /* Insert cleanup code */
    for (i = 0, p = l; p; i++) {
      if ((tm = Getattr(p, "tmap:freearg"))) {
	Replaceall(tm, "$source", Getattr(p, "lname"));
	Replaceall(tm, "$arg", Getattr(p, "emit:input"));
	Replaceall(tm, "$input", Getattr(p, "emit:input"));
	Printv(cleanup, tm, "\n", NIL);
	p = Getattr(p, "tmap:freearg:next");
      } else {
	p = nextSibling(p);
      }
    }

    /* Insert argument output code */
    num_saved = 0;
    for (i = 0, p = l; p; i++) {
      if ((tm = Getattr(p, "tmap:argout"))) {
	SwigType *t = Getattr(p, "type");
	Replaceall(tm, "$source", Getattr(p, "lname"));
	Replaceall(tm, "$target", "ST(argvi)");
	Replaceall(tm, "$result", "ST(argvi)");
	if (is_shadow(t)) {
	  Replaceall(tm, "$shadow", "SWIG_SHADOW");
	} else {
	  Replaceall(tm, "$shadow", "0");
	}

	String *in = Getattr(p, "emit:input");
	if (in) {
	  sprintf(temp, "_saved[%d]", num_saved);
	  Replaceall(tm, "$arg", temp);
	  Replaceall(tm, "$input", temp);
	  Printf(f->code, "_saved[%d] = %s;\n", num_saved, in);
	  num_saved++;
	}
	Printv(outarg, tm, "\n", NIL);
	p = Getattr(p, "tmap:argout:next");
      } else {
	p = nextSibling(p);
      }
    }

    /* If there were any saved arguments, emit a local variable for them */
    if (num_saved) {
      sprintf(temp, "_saved[%d]", num_saved);
      Wrapper_add_localv(f, "_saved", "SV *", temp, NIL);
    }

    /* Now write code to make the function call */

    emit_action(n, f);

    if ((tm = Swig_typemap_lookup_new("out", n, "result", 0))) {
      SwigType *t = Getattr(n, "type");
      Replaceall(tm, "$source", "result");
      Replaceall(tm, "$target", "ST(argvi)");
      Replaceall(tm, "$result", "ST(argvi)");
      if (is_shadow(t)) {
	Replaceall(tm, "$shadow", "SWIG_SHADOW");
      } else {
	Replaceall(tm, "$shadow", "0");
      }
      if (GetFlag(n, "feature:new")) {
	Replaceall(tm, "$owner", "SWIG_OWNER");
      } else {
	Replaceall(tm, "$owner", "0");
      }
      Printf(f->code, "%s\n", tm);
    } else {
      Swig_warning(WARN_TYPEMAP_OUT_UNDEF, input_file, line_number, "Unable to use return type %s in function %s.\n", SwigType_str(d, 0), name);
    }

    /* If there were any output args, take care of them. */

    Printv(f->code, outarg, NIL);

    /* If there was any cleanup, do that. */

    Printv(f->code, cleanup, NIL);

    if (GetFlag(n, "feature:new")) {
      if ((tm = Swig_typemap_lookup_new("newfree", n, "result", 0))) {
	Replaceall(tm, "$source", "result");
	Printf(f->code, "%s\n", tm);
      }
    }

    if ((tm = Swig_typemap_lookup_new("ret", n, "result", 0))) {
      Replaceall(tm, "$source", "result");
      Printf(f->code, "%s\n", tm);
    }

    Printv(f->code, "XSRETURN(argvi);\n", "fail:\n", cleanup, "SWIG_croak_null();\n" "}\n" "}\n", NIL);

    /* Add the dXSARGS last */

    Wrapper_add_local(f, "dXSARGS", "dXSARGS");

    /* Substitute the cleanup code */
    Replaceall(f->code, "$cleanup", cleanup);
    Replaceall(f->code, "$symname", iname);

    /* Dump the wrapper function */

    Wrapper_print(f, f_wrappers);

    /* Now register the function */

    if (!Getattr(n, "sym:overloaded")) {
      Printf(command_tab, "{\"%s::%s\", %s},\n", cmodule, iname, wname);
    } else if (!Getattr(n, "sym:nextSibling")) {
      /* Generate overloaded dispatch function */
      int maxargs;
      String *dispatch = Swig_overload_dispatch_cast(n, "++PL_markstack_ptr; SWIG_CALLXS(%s); return;", &maxargs);

      /* Generate a dispatch wrapper for all overloaded functions */

      Wrapper *df = NewWrapper();
      String *dname = Swig_name_wrapper(iname);

      Printv(df->def, "XS(", dname, ") {\n", NIL);

      Wrapper_add_local(df, "dXSARGS", "dXSARGS");
      Printv(df->code, dispatch, "\n", NIL);
      Printf(df->code, "croak(\"No matching function for overloaded '%s'\");\n", iname);
      Printf(df->code, "XSRETURN(0);\n");
      Printv(df->code, "}\n", NIL);
      Wrapper_print(df, f_wrappers);
      Printf(command_tab, "{\"%s::%s\", %s},\n", cmodule, iname, dname);
      DelWrapper(df);
      Delete(dispatch);
      Delete(dname);
    }
    if (!Getattr(n, "sym:nextSibling")) {
      if (export_all) {
	Printf(exported, "%s ", iname);
      }

      /* --------------------------------------------------------------------
       * Create a stub for this function, provided it's not a member function
       * -------------------------------------------------------------------- */

      if ((blessed) && (!member_func)) {
	Printv(func_stubs, "*", iname, " = *", cmodule, "::", iname, ";\n", NIL);
      }

    }
    Delete(cleanup);
    Delete(outarg);
    DelWrapper(f);
    return SWIG_OK;
  }

  /* ------------------------------------------------------------
   * variableWrapper()
   * ------------------------------------------------------------ */
  virtual int variableWrapper(Node *n) {
    String *name = Getattr(n, "name");
    String *iname = Getattr(n, "sym:name");
    SwigType *t = Getattr(n, "type");
    Wrapper *getf, *setf;
    String *tm;

    String *set_name = Swig_name_wrapper(Swig_name_set(iname));
    String *val_name = Swig_name_wrapper(Swig_name_get(iname));

    if (!addSymbol(iname, n))
      return SWIG_ERROR;

    getf = NewWrapper();
    setf = NewWrapper();

    /* Create a Perl function for setting the variable value */

    if (!GetFlag(n, "feature:immutable")) {
      Printf(setf->def, "SWIGCLASS_STATIC int %s(pTHX_ SV* sv, MAGIC * SWIGUNUSEDPARM(mg)) {\n", set_name);
      Printv(setf->code, tab4, "MAGIC_PPERL\n", NIL);

      /* Check for a few typemaps */
      tm = Swig_typemap_lookup_new("varin", n, name, 0);
      if (tm) {
	Replaceall(tm, "$source", "sv");
	Replaceall(tm, "$target", name);
	Replaceall(tm, "$input", "sv");
	/* Printf(setf->code,"%s\n", tm); */
	emit_action_code(n, setf, tm);
      } else {
	Swig_warning(WARN_TYPEMAP_VARIN_UNDEF, input_file, line_number, "Unable to set variable of type %s.\n", SwigType_str(t, 0));
	return SWIG_NOWRAP;
      }
      Printf(setf->code, "fail:\n");
      Printf(setf->code, "    return 1;\n}\n");
      Replaceall(setf->code, "$symname", iname);
      Wrapper_print(setf, magic);
    }

    /* Now write a function to evaluate the variable */
    int addfail = 0;
    Printf(getf->def, "SWIGCLASS_STATIC int %s(pTHX_ SV *sv, MAGIC *SWIGUNUSEDPARM(mg)) {\n", val_name);
    Printv(getf->code, tab4, "MAGIC_PPERL\n", NIL);

    if ((tm = Swig_typemap_lookup_new("varout", n, name, 0))) {
      Replaceall(tm, "$target", "sv");
      Replaceall(tm, "$result", "sv");
      Replaceall(tm, "$source", name);
      if (is_shadow(t)) {
	Replaceall(tm, "$shadow", "SWIG_SHADOW");
      } else {
	Replaceall(tm, "$shadow", "0");
      }
      /* Printf(getf->code,"%s\n", tm); */
      addfail = emit_action_code(n, getf, tm);
    } else {
      Swig_warning(WARN_TYPEMAP_VAROUT_UNDEF, input_file, line_number, "Unable to read variable of type %s\n", SwigType_str(t, 0));
      return SWIG_NOWRAP;
    }
    Printf(getf->code, "    return 1;\n");
    if (addfail) {
      Append(getf->code, "fail:\n");
      Append(getf->code, "  return 0;\n");
    }
    Append(getf->code, "}\n");


    Replaceall(getf->code, "$symname", iname);
    Wrapper_print(getf, magic);

    String *tt = Getattr(n, "tmap:varout:type");
    if (tt) {
      String *tm = NewStringf("&SWIGTYPE%s", SwigType_manglestr(t));
      if (Replaceall(tt, "$1_descriptor", tm)) {
	SwigType_remember(t);
      }
      Delete(tm);
      SwigType *st = Copy(t);
      SwigType_add_pointer(st);
      tm = NewStringf("&SWIGTYPE%s", SwigType_manglestr(st));
      if (Replaceall(tt, "$&1_descriptor", tm)) {
	SwigType_remember(st);
      }
      Delete(tm);
      Delete(st);
    } else {
      tt = (String *) "0";
    }
    /* Now add symbol to the PERL interpreter */
    if (GetFlag(n, "feature:immutable")) {
      Printv(variable_tab, tab4, "{ \"", cmodule, "::", iname, "\", MAGIC_CLASS swig_magic_readonly, MAGIC_CLASS ", val_name, ",", tt, " },\n", NIL);

    } else {
      Printv(variable_tab, tab4, "{ \"", cmodule, "::", iname, "\", MAGIC_CLASS ", set_name, ", MAGIC_CLASS ", val_name, ",", tt, " },\n", NIL);
    }

    /* If we're blessed, try to figure out what to do with the variable
       1.  If it's a Perl object of some sort, create a tied-hash
       around it.
       2.  Otherwise, just hack Perl's symbol table */

    if (blessed) {
      if (is_shadow(t)) {
	Printv(var_stubs,
	       "\nmy %__", iname, "_hash;\n",
	       "tie %__", iname, "_hash,\"", is_shadow(t), "\", $",
	       cmodule, "::", iname, ";\n", "$", iname, "= \\%__", iname, "_hash;\n", "bless $", iname, ", ", is_shadow(t), ";\n", NIL);
      } else {
	Printv(var_stubs, "*", iname, " = *", cmodule, "::", iname, ";\n", NIL);
      }
    }
    if (export_all)
      Printf(exported, "$%s ", iname);

    DelWrapper(setf);
    DelWrapper(getf);
    Delete(set_name);
    Delete(val_name);
    return SWIG_OK;
  }

  /* ------------------------------------------------------------
   * constantWrapper()
   * ------------------------------------------------------------ */

  virtual int constantWrapper(Node *n) {
    String *name = Getattr(n, "name");
    String *iname = Getattr(n, "sym:name");
    SwigType *type = Getattr(n, "type");
    String *rawval = Getattr(n, "rawval");
    String *value = rawval ? rawval : Getattr(n, "value");
    String *tm;

    if (!addSymbol(iname, n))
      return SWIG_ERROR;

    /* Special hook for member pointer */
    if (SwigType_type(type) == T_MPOINTER) {
      String *wname = Swig_name_wrapper(iname);
      Printf(f_wrappers, "static %s = %s;\n", SwigType_str(type, wname), value);
      value = Char(wname);
    }

    if ((tm = Swig_typemap_lookup_new("consttab", n, name, 0))) {
      Replaceall(tm, "$source", value);
      Replaceall(tm, "$target", name);
      Replaceall(tm, "$value", value);
      if (is_shadow(type)) {
	Replaceall(tm, "$shadow", "SWIG_SHADOW");
      } else {
	Replaceall(tm, "$shadow", "0");
      }
      Printf(constant_tab, "%s,\n", tm);
    } else if ((tm = Swig_typemap_lookup_new("constcode", n, name, 0))) {
      Replaceall(tm, "$source", value);
      Replaceall(tm, "$target", name);
      Replaceall(tm, "$value", value);
      if (is_shadow(type)) {
	Replaceall(tm, "$shadow", "SWIG_SHADOW");
      } else {
	Replaceall(tm, "$shadow", "0");
      }
      Printf(f_init, "%s\n", tm);
    } else {
      Swig_warning(WARN_TYPEMAP_CONST_UNDEF, input_file, line_number, "Unsupported constant value.\n");
      return SWIG_NOWRAP;
    }

    if (blessed) {
      if (is_shadow(type)) {
	Printv(var_stubs,
	       "\nmy %__", iname, "_hash;\n",
	       "tie %__", iname, "_hash,\"", is_shadow(type), "\", $",
	       cmodule, "::", iname, ";\n", "$", iname, "= \\%__", iname, "_hash;\n", "bless $", iname, ", ", is_shadow(type), ";\n", NIL);
      } else if (do_constants) {
	Printv(const_stubs, "sub ", name, " () { $", cmodule, "::", name, " }\n", NIL);
	num_consts++;
      } else {
	Printv(var_stubs, "*", iname, " = *", cmodule, "::", iname, ";\n", NIL);
      }
    }
    if (export_all) {
      if (do_constants && !is_shadow(type)) {
	Printf(exported, "%s ", name);
      } else {
	Printf(exported, "$%s ", iname);
      }
    }
    return SWIG_OK;
  }

  /* ------------------------------------------------------------
   * usage_func()
   * ------------------------------------------------------------ */
  char *usage_func(char *iname, SwigType *, ParmList *l) {
    static String *temp = 0;
    Parm *p;
    int i;

    if (!temp)
      temp = NewString("");
    Clear(temp);
    Printf(temp, "%s(", iname);

    /* Now go through and print parameters */
    p = l;
    i = 0;
    while (p != 0) {
      SwigType *pt = Getattr(p, "type");
      String *pn = Getattr(p, "name");
      if (!Getattr(p, "ignore")) {
	/* If parameter has been named, use that.   Otherwise, just print a type  */
	if (SwigType_type(pt) != T_VOID) {
	  if (Len(pn) > 0) {
	    Printf(temp, "%s", pn);
	  } else {
	    Printf(temp, "%s", SwigType_str(pt, 0));
	  }
	}
	i++;
	p = nextSibling(p);
	if (p)
	  if (!Getattr(p, "ignore"))
	    Putc(',', temp);
      } else {
	p = nextSibling(p);
	if (p)
	  if ((i > 0) && (!Getattr(p, "ignore")))
	    Putc(',', temp);
      }
    }
    Printf(temp, ");");
    return Char(temp);
  }

  /* ------------------------------------------------------------
   * nativeWrapper()
   * ------------------------------------------------------------ */

  virtual int nativeWrapper(Node *n) {
    String *name = Getattr(n, "sym:name");
    String *funcname = Getattr(n, "wrap:name");

    if (!addSymbol(funcname, n))
      return SWIG_ERROR;

    Printf(command_tab, "{\"%s::%s\", %s},\n", cmodule, name, funcname);
    if (export_all)
      Printf(exported, "%s ", name);
    if (blessed) {
      Printv(func_stubs, "*", name, " = *", cmodule, "::", name, ";\n", NIL);
    }
    return SWIG_OK;
  }

/* ----------------------------------------------------------------------------
 *                      OBJECT-ORIENTED FEATURES
 *
 * These extensions provide a more object-oriented interface to C++
 * classes and structures.    The code here is based on extensions
 * provided by David Fletcher and Gary Holt.
 *
 * I have generalized these extensions to make them more general purpose
 * and to resolve object-ownership problems.
 *
 * The approach here is very similar to the Python module :
 *       1.   All of the original methods are placed into a single
 *            package like before except that a 'c' is appended to the
 *            package name.
 *
 *       2.   All methods and function calls are wrapped with a new
 *            perl function.   While possibly inefficient this allows
 *            us to catch complex function arguments (which are hard to
 *            track otherwise).
 *
 *       3.   Classes are represented as tied-hashes in a manner similar
 *            to Gary Holt's extension.   This allows us to access
 *            member data.
 *
 *       4.   Stand-alone (global) C functions are modified to take
 *            tied hashes as arguments for complex datatypes (if
 *            appropriate).
 *
 *       5.   Global variables involving a class/struct is encapsulated
 *            in a tied hash.
 *
 * ------------------------------------------------------------------------- */


  void setclassname(Node *n) {
    String *symname = Getattr(n, "sym:name");
    String *fullname;
    String *actualpackage;
    Node *clsmodule = Getattr(n, "module");

    if (!clsmodule) {
      /* imported module does not define a module name.   Oh well */
      return;
    }

    /* Do some work on the class name */
    actualpackage = Getattr(clsmodule, "name");
    if ((!compat) && (!Strchr(symname, ':'))) {
      fullname = NewStringf("%s::%s", actualpackage, symname);
    } else {
      fullname = NewString(symname);
    }
    Setattr(n, "perl5:proxy", fullname);
  }

  /* ------------------------------------------------------------
   * classDeclaration()
   * ------------------------------------------------------------ */
  virtual int classDeclaration(Node *n) {
    /* Do some work on the class name */
    if (!Getattr(n, "feature:onlychildren")) {
      if (blessed) {
	setclassname(n);
	Append(classlist, n);
      }
    }

    return Language::classDeclaration(n);
  }

  /* ------------------------------------------------------------
   * classHandler()
   * ------------------------------------------------------------ */

  virtual int classHandler(Node *n) {

    if (blessed) {
      have_constructor = 0;
      have_operators = 0;
      have_destructor = 0;
      have_data_members = 0;
      operators = NewHash();

      class_name = Getattr(n, "sym:name");

      if (!addSymbol(class_name, n))
	return SWIG_ERROR;

      /* Use the fully qualified name of the Perl class */
      if (!compat) {
	fullclassname = NewStringf("%s::%s", fullmodule, class_name);
      } else {
	fullclassname = NewString(class_name);
      }
      real_classname = Getattr(n, "name");
      pcode = NewString("");
      // blessedmembers = NewString("");
    }

    /* Emit all of the members */
    Language::classHandler(n);


    /* Finish the rest of the class */
    if (blessed) {
      /* Generate a client-data entry */
      SwigType *ct = NewStringf("p.%s", real_classname);
      Printv(f_init, "SWIG_TypeClientData(SWIGTYPE", SwigType_manglestr(ct), ", (void*) \"", fullclassname, "\");\n", NIL);
      SwigType_remember(ct);
      Delete(ct);

      Printv(pm, "\n############# Class : ", fullclassname, " ##############\n", "\npackage ", fullclassname, ";\n", NIL);

      if (have_operators) {
	Printf(pm, "use overload\n");
	Iterator ki;
	for (ki = First(operators); ki.key; ki = Next(ki)) {
	  char *name = Char(ki.key);
	  //        fprintf(stderr,"found name: <%s>\n", name);
	  if (strstr(name, "__eq__")) {
	    Printv(pm, tab4, "\"==\" => sub { $_[0]->__eq__($_[1])},\n", NIL);
	  } else if (strstr(name, "__ne__")) {
	    Printv(pm, tab4, "\"!=\" => sub { $_[0]->__ne__($_[1])},\n", NIL);
	  } else if (strstr(name, "__assign__")) {
	    Printv(pm, tab4, "\"=\" => sub { $_[0]->__assign__($_[1])},\n", NIL);
	  } else if (strstr(name, "__str__")) {
	    Printv(pm, tab4, "'\"\"' => sub { $_[0]->__str__()},\n", NIL);
	  } else if (strstr(name, "__plusplus__")) {
	    Printv(pm, tab4, "\"++\" => sub { $_[0]->__plusplus__()},\n", NIL);
	  } else if (strstr(name, "__minmin__")) {
	    Printv(pm, tab4, "\"--\" => sub { $_[0]->__minmin__()},\n", NIL);
	  } else if (strstr(name, "__add__")) {
	    Printv(pm, tab4, "\"+\" => sub { $_[0]->__add__($_[1])},\n", NIL);
	  } else if (strstr(name, "__sub__")) {
	    Printv(pm, tab4, "\"-\" => sub { $_[0]->__sub__($_[1])},\n", NIL);
	  } else if (strstr(name, "__mul__")) {
	    Printv(pm, tab4, "\"*\" => sub { $_[0]->__mul__($_[1])},\n", NIL);
	  } else if (strstr(name, "__div__")) {
	    Printv(pm, tab4, "\"/\" => sub { $_[0]->__div__($_[1])},\n", NIL);
	  } else if (strstr(name, "__mod__")) {
	    Printv(pm, tab4, "\"%\" => sub { $_[0]->__mod__($_[1])},\n", NIL);
	  } else if (strstr(name, "__and__")) {
	    Printv(pm, tab4, "\"&\" => sub { $_[0]->__and__($_[1])},\n", NIL);
	  } else if (strstr(name, "__or__")) {
	    Printv(pm, tab4, "\"|\" => sub { $_[0]->__or__($_[1])},\n", NIL);
	  } else if (strstr(name, "__gt__")) {
	    Printv(pm, tab4, "\">\" => sub { $_[0]->__gt__($_[1])},\n", NIL);
	  } else if (strstr(name, "__ge__")) {
	    Printv(pm, tab4, "\">=\" => sub { $_[0]->__ge__($_[1])},\n", NIL);
	  } else if (strstr(name, "__not__")) {
	    Printv(pm, tab4, "\"!\" => sub { $_[0]->__not__()},\n", NIL);
	  } else if (strstr(name, "__lt__")) {
	    Printv(pm, tab4, "\"<\" => sub { $_[0]->__lt__($_[1])},\n", NIL);
	  } else if (strstr(name, "__le__")) {
	    Printv(pm, tab4, "\"<=\" => sub { $_[0]->__le__($_[1])},\n", NIL);
	  } else if (strstr(name, "__pluseq__")) {
	    Printv(pm, tab4, "\"+=\" => sub { $_[0]->__pluseq__($_[1])},\n", NIL);
	  } else if (strstr(name, "__mineq__")) {
	    Printv(pm, tab4, "\"-=\" => sub { $_[0]->__mineq__($_[1])},\n", NIL);
	  } else if (strstr(name, "__neg__")) {
	    Printv(pm, tab4, "\"neg\" => sub { $_[0]->__neg__()},\n", NIL);
	  } else {
	    fprintf(stderr, "Unknown operator: %s\n", name);
	  }
	}
	Printv(pm, tab4, "\"fallback\" => 1;\n", NIL);
      }
      // make use strict happy
      Printv(pm, "use vars qw(@ISA %OWNER %ITERATORS %BLESSEDMEMBERS);\n", NIL);

      /* If we are inheriting from a base class, set that up */

      Printv(pm, "@ISA = qw(", NIL);

      /* Handle inheritance */
      List *baselist = Getattr(n, "bases");
      if (baselist && Len(baselist)) {
	Iterator b;
	b = First(baselist);
	while (b.item) {
	  String *bname = Getattr(b.item, "perl5:proxy");
	  if (!bname) {
	    b = Next(b);
	    continue;
	  }
	  Printv(pm, " ", bname, NIL);
	  b = Next(b);
	}
      }

      /* Module comes last */
      if (!compat || Cmp(fullmodule, fullclassname)) {
	Printv(pm, " ", fullmodule, NIL);
      }

      Printf(pm, " );\n");

      /* Dump out a hash table containing the pointers that we own */
      Printf(pm, "%%OWNER = ();\n");
      if (have_data_members || have_destructor)
	Printf(pm, "%%ITERATORS = ();\n");

      /* Dump out the package methods */

      Printv(pm, pcode, NIL);
      Delete(pcode);

      /* Output methods for managing ownership */

      Printv(pm,
	     "sub DISOWN {\n",
	     tab4, "my $self = shift;\n",
	     tab4, "my $ptr = tied(%$self);\n",
	     tab4, "delete $OWNER{$ptr};\n",
	     "}\n\n", "sub ACQUIRE {\n", tab4, "my $self = shift;\n", tab4, "my $ptr = tied(%$self);\n", tab4, "$OWNER{$ptr} = 1;\n", "}\n\n", NIL);

      /* Only output the following methods if a class has member data */

      Delete(operators);
      operators = 0;
    }
    return SWIG_OK;
  }

  /* ------------------------------------------------------------
   * memberfunctionHandler()
   * ------------------------------------------------------------ */

  virtual int memberfunctionHandler(Node *n) {
    String *symname = Getattr(n, "sym:name");

    member_func = 1;
    Language::memberfunctionHandler(n);
    member_func = 0;

    if ((blessed) && (!Getattr(n, "sym:nextSibling"))) {

      if (Strstr(symname, "__eq__")) {
	DohSetInt(operators, "__eq__", 1);
	have_operators = 1;
      } else if (Strstr(symname, "__ne__")) {
	DohSetInt(operators, "__ne__", 1);
	have_operators = 1;
      } else if (Strstr(symname, "__assign__")) {
	DohSetInt(operators, "__assign__", 1);
	have_operators = 1;
      } else if (Strstr(symname, "__str__")) {
	DohSetInt(operators, "__str__", 1);
	have_operators = 1;
      } else if (Strstr(symname, "__add__")) {
	DohSetInt(operators, "__add__", 1);
	have_operators = 1;
      } else if (Strstr(symname, "__sub__")) {
	DohSetInt(operators, "__sub__", 1);
	have_operators = 1;
      } else if (Strstr(symname, "__mul__")) {
	DohSetInt(operators, "__mul__", 1);
	have_operators = 1;
      } else if (Strstr(symname, "__div__")) {
	DohSetInt(operators, "__div__", 1);
	have_operators = 1;
      } else if (Strstr(symname, "__mod__")) {
	DohSetInt(operators, "__mod__", 1);
	have_operators = 1;
      } else if (Strstr(symname, "__and__")) {
	DohSetInt(operators, "__and__", 1);
	have_operators = 1;
      } else if (Strstr(symname, "__or__")) {
	DohSetInt(operators, "__or__", 1);
	have_operators = 1;
      } else if (Strstr(symname, "__not__")) {
	DohSetInt(operators, "__not__", 1);
	have_operators = 1;
      } else if (Strstr(symname, "__gt__")) {
	DohSetInt(operators, "__gt__", 1);
	have_operators = 1;
      } else if (Strstr(symname, "__ge__")) {
	DohSetInt(operators, "__ge__", 1);
	have_operators = 1;
      } else if (Strstr(symname, "__lt__")) {
	DohSetInt(operators, "__lt__", 1);
	have_operators = 1;
      } else if (Strstr(symname, "__le__")) {
	DohSetInt(operators, "__le__", 1);
	have_operators = 1;
      } else if (Strstr(symname, "__neg__")) {
	DohSetInt(operators, "__neg__", 1);
	have_operators = 1;
      } else if (Strstr(symname, "__plusplus__")) {
	DohSetInt(operators, "__plusplus__", 1);
	have_operators = 1;
      } else if (Strstr(symname, "__minmin__")) {
	DohSetInt(operators, "__minmin__", 1);
	have_operators = 1;
      } else if (Strstr(symname, "__mineq__")) {
	DohSetInt(operators, "__mineq__", 1);
	have_operators = 1;
      } else if (Strstr(symname, "__pluseq__")) {
	DohSetInt(operators, "__pluseq__", 1);
	have_operators = 1;
      }

      if (Getattr(n, "feature:shadow")) {
	String *plcode = perlcode(Getattr(n, "feature:shadow"), 0);
	String *plaction = NewStringf("%s::%s", cmodule, Swig_name_member(class_name, symname));
	Replaceall(plcode, "$action", plaction);
	Delete(plaction);
	Printv(pcode, plcode, NIL);
      } else {
	Printv(pcode, "*", symname, " = *", cmodule, "::", Swig_name_member(class_name, symname), ";\n", NIL);
      }
    }
    return SWIG_OK;
  }

  /* ------------------------------------------------------------
   * membervariableHandler()
   *
   * Adds an instance member.
   * ----------------------------------------------------------------------------- */

  virtual int membervariableHandler(Node *n) {

    String *symname = Getattr(n, "sym:name");
    /* SwigType *t  = Getattr(n,"type"); */

    /* Emit a pair of get/set functions for the variable */

    member_func = 1;
    Language::membervariableHandler(n);
    member_func = 0;

    if (blessed) {

      Printv(pcode, "*swig_", symname, "_get = *", cmodule, "::", Swig_name_get(Swig_name_member(class_name, symname)), ";\n", NIL);
      Printv(pcode, "*swig_", symname, "_set = *", cmodule, "::", Swig_name_set(Swig_name_member(class_name, symname)), ";\n", NIL);

      /* Now we need to generate a little Perl code for this */

      /* if (is_shadow(t)) {

       *//* This is a Perl object that we have already seen.  Add an
         entry to the members list *//*
         Printv(blessedmembers,
         tab4, symname, " => '", is_shadow(t), "',\n",
         NIL);

         }
       */
    }
    have_data_members++;
    return SWIG_OK;
  }

  /* ------------------------------------------------------------
   * constructorDeclaration()
   *
   * Emits a blessed constructor for our class.    In addition to our construct
   * we manage a Perl hash table containing all of the pointers created by
   * the constructor.   This prevents us from accidentally trying to free
   * something that wasn't necessarily allocated by malloc or new
   * ------------------------------------------------------------ */

  virtual int constructorHandler(Node *n) {

    String *symname = Getattr(n, "sym:name");

    member_func = 1;
    Language::constructorHandler(n);

    if ((blessed) && (!Getattr(n, "sym:nextSibling"))) {
      if (Getattr(n, "feature:shadow")) {
	String *plcode = perlcode(Getattr(n, "feature:shadow"), 0);
	String *plaction = NewStringf("%s::%s", module, Swig_name_member(class_name, symname));
	Replaceall(plcode, "$action", plaction);
	Delete(plaction);
	Printv(pcode, plcode, NIL);
      } else {
	if ((Cmp(symname, class_name) == 0)) {
	  /* Emit a blessed constructor  */
	  Printf(pcode, "sub new {\n");
	} else {
	  /* Constructor doesn't match classname so we'll just use the normal name  */
	  Printv(pcode, "sub ", Swig_name_construct(symname), " () {\n", NIL);
	}

	Printv(pcode,
	       tab4, "my $pkg = shift;\n",
	       tab4, "my $self = ", cmodule, "::", Swig_name_construct(symname), "(@_);\n", tab4, "bless $self, $pkg if defined($self);\n", "}\n\n", NIL);

	have_constructor = 1;
      }
    }
    member_func = 0;
    return SWIG_OK;
  }

  /* ------------------------------------------------------------ 
   * destructorHandler()
   * ------------------------------------------------------------ */

  virtual int destructorHandler(Node *n) {
    String *symname = Getattr(n, "sym:name");
    member_func = 1;
    Language::destructorHandler(n);
    if (blessed) {
      if (Getattr(n, "feature:shadow")) {
	String *plcode = perlcode(Getattr(n, "feature:shadow"), 0);
	String *plaction = NewStringf("%s::%s", module, Swig_name_member(class_name, symname));
	Replaceall(plcode, "$action", plaction);
	Delete(plaction);
	Printv(pcode, plcode, NIL);
      } else {
	Printv(pcode,
	       "sub DESTROY {\n",
	       tab4, "return unless $_[0]->isa('HASH');\n",
	       tab4, "my $self = tied(%{$_[0]});\n",
	       tab4, "return unless defined $self;\n",
	       tab4, "delete $ITERATORS{$self};\n",
	       tab4, "if (exists $OWNER{$self}) {\n",
	       tab8, cmodule, "::", Swig_name_destroy(symname), "($self);\n", tab8, "delete $OWNER{$self};\n", tab4, "}\n}\n\n", NIL);
	have_destructor = 1;
      }
    }
    member_func = 0;
    return SWIG_OK;
  }

  /* ------------------------------------------------------------
   * staticmemberfunctionHandler()
   * ------------------------------------------------------------ */

  virtual int staticmemberfunctionHandler(Node *n) {
    member_func = 1;
    Language::staticmemberfunctionHandler(n);
    member_func = 0;
    if ((blessed) && (!Getattr(n, "sym:nextSibling"))) {
      String *symname = Getattr(n, "sym:name");
      Printv(pcode, "*", symname, " = *", cmodule, "::", Swig_name_member(class_name, symname), ";\n", NIL);
    }
    return SWIG_OK;
  }

  /* ------------------------------------------------------------
   * staticmembervariableHandler()
   * ------------------------------------------------------------ */

  virtual int staticmembervariableHandler(Node *n) {
    Language::staticmembervariableHandler(n);
    if (blessed) {
      String *symname = Getattr(n, "sym:name");
      Printv(pcode, "*", symname, " = *", cmodule, "::", Swig_name_member(class_name, symname), ";\n", NIL);
    }
    return SWIG_OK;
  }

  /* ------------------------------------------------------------
   * memberconstantHandler()
   * ------------------------------------------------------------ */

  virtual int memberconstantHandler(Node *n) {
    String *symname = Getattr(n, "sym:name");
    int oldblessed = blessed;

    /* Create a normal constant */
    blessed = 0;
    Language::memberconstantHandler(n);
    blessed = oldblessed;

    if (blessed) {
      Printv(pcode, "*", symname, " = *", cmodule, "::", Swig_name_member(class_name, symname), ";\n", NIL);
    }
    return SWIG_OK;
  }

  /* ------------------------------------------------------------
   * pragma()
   *
   * Pragma directive.
   *
   * %pragma(perl5) code="String"              # Includes a string in the .pm file
   * %pragma(perl5) include="file.pl"          # Includes a file in the .pm file
   * ------------------------------------------------------------ */

  virtual int pragmaDirective(Node *n) {
    String *lang;
    String *code;
    String *value;
    if (!ImportMode) {
      lang = Getattr(n, "lang");
      code = Getattr(n, "name");
      value = Getattr(n, "value");
      if (Strcmp(lang, "perl5") == 0) {
	if (Strcmp(code, "code") == 0) {
	  /* Dump the value string into the .pm file */
	  if (value) {
	    Printf(pragma_include, "%s\n", value);
	  }
	} else if (Strcmp(code, "include") == 0) {
	  /* Include a file into the .pm file */
	  if (value) {
	    FILE *f = Swig_open(value);
	    if (!f) {
	      Printf(stderr, "%s : Line %d. Unable to locate file %s\n", input_file, line_number, value);
	    } else {
	      char buffer[4096];
	      while (fgets(buffer, 4095, f)) {
		Printf(pragma_include, "%s", buffer);
	      }
	    }
	  }
	} else {
	  Printf(stderr, "%s : Line %d. Unrecognized pragma.\n", input_file, line_number);
	}
      }
    }
    return Language::pragmaDirective(n);
  }

  /* ------------------------------------------------------------
   * perlcode()     - Output perlcode code into the shadow file
   * ------------------------------------------------------------ */

  String *perlcode(String *code, const String *indent) {
    String *out = NewString("");
    String *temp;
    char *t;
    if (!indent)
      indent = "";

    temp = NewString(code);

    t = Char(temp);
    if (*t == '{') {
      Delitem(temp, 0);
      Delitem(temp, DOH_END);
    }

    /* Split the input text into lines */
    List *clist = DohSplitLines(temp);
    Delete(temp);
    int initial = 0;
    String *s = 0;
    Iterator si;
    /* Get the initial indentation */

    for (si = First(clist); si.item; si = Next(si)) {
      s = si.item;
      if (Len(s)) {
	char *c = Char(s);
	while (*c) {
	  if (!isspace(*c))
	    break;
	  initial++;
	  c++;
	}
	if (*c && !isspace(*c))
	  break;
	else {
	  initial = 0;
	}
      }
    }
    while (si.item) {
      s = si.item;
      if (Len(s) > initial) {
	char *c = Char(s);
	c += initial;
	Printv(out, indent, c, "\n", NIL);
      } else {
	Printv(out, "\n", NIL);
      }
      si = Next(si);
    }
    Delete(clist);
    return out;
  }

  /* ------------------------------------------------------------
   * insertDirective()
   * 
   * Hook for %insert directive.
   * ------------------------------------------------------------ */

  virtual int insertDirective(Node *n) {
    String *code = Getattr(n, "code");
    String *section = Getattr(n, "section");

    if ((!ImportMode) && (Cmp(section, "perl") == 0)) {
      Printv(additional_perl_code, code, NIL);
    } else {
      Language::insertDirective(n);
    }
    return SWIG_OK;
  }

  String *runtimeCode() {
    String *s = NewString("");
    String *shead = Swig_include_sys("perlhead.swg");
    if (!shead) {
      Printf(stderr, "*** Unable to open 'perlhead.swg'\n");
    } else {
      Append(s, shead);
      Delete(shead);
    }
    String *serrors = Swig_include_sys("perlerrors.swg");
    if (!serrors) {
      Printf(stderr, "*** Unable to open 'perlerrors.swg'\n");
    } else {
      Append(s, serrors);
      Delete(serrors);
    }
    String *srun = Swig_include_sys("perlrun.swg");
    if (!srun) {
      Printf(stderr, "*** Unable to open 'perlrun.swg'\n");
    } else {
      Append(s, srun);
      Delete(srun);
    }
    return s;
  }

  String *defaultExternalRuntimeFilename() {
    return NewString("swigperlrun.h");
  }
};

/* -----------------------------------------------------------------------------
 * swig_perl5()    - Instantiate module
 * ----------------------------------------------------------------------------- */

static Language *new_swig_perl5() {
  return new PERL5();
}
extern "C" Language *swig_perl5(void) {
  return new_swig_perl5();
}
