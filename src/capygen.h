struct hashmap *capy_identifiers_ht = 0;
u8 *capy_identifiers_buf = 0;
void *capy_identifiers_ptr = 0;
void *capy_identifiers_end = 0;
struct hashmap *capy_signatures = 0;

#define CAPY_IDENT_BUFSIZE 1024 * 32
#define CAPY_MAX_PARAMS 16

u8 *capy_create_identifier(char *s) {
	u8 *result;
	u32 len;
	u64 hash;
	// lazy init
	if (!capy_identifiers_ht) {
		capy_identifiers_ht = hashmap_create();
		capy_identifiers_buf = mem_alloc(CAPY_IDENT_BUFSIZE);
		memset(capy_identifiers_buf, 0, CAPY_IDENT_BUFSIZE);
		capy_identifiers_ptr = capy_identifiers_buf;
		capy_identifiers_end = capy_identifiers_buf + CAPY_IDENT_BUFSIZE;
	}
	hash = char_hash(s);
	result = hashmap_get(capy_identifiers_ht, hash);
	if (!result) {
		len = strlen(s) + 1;
		if (capy_identifiers_ptr + len >= capy_identifiers_end)
			tcc_error("capy identifiers buffer overflow");
		result = capy_identifiers_ptr;
		capy_identifiers_ptr += len;
		strcpy(result, s);
		hashmap_insert(capy_identifiers_ht, hash, result);
		// printf("created identifier: %s\n", result);
	}
	return result;
}

typedef struct LinkedList {

	struct LinkedList* next;
	union {
		int64_t num;
		void* ptr;
		char chars[8];
	} val;

} LinkedList;

typedef struct VarSignature {
	u32 type_hash;
	u32 name_hash;
	u8 *type_name;
	u8 *human_name;
	u8 size;
	u32 array_size;
	CType* ctype;
	int tok;
	Sym* s;
	u8 is_basic_type : 1;
	u8 is_func : 1;
	u8 is_signed : 1;
	u8 is_float : 1;
	u8 is_void : 1;
	u8 is_bool : 1;
	u8 is_enum : 1;
	u8 is_pointer : 1;
	u8 is_struct : 1;
	u8 is_array : 1;
	u8 is_const : 1;
	u8 is_volatile : 1;
	/*struct VarSignature *next_param;
	struct VarSignature *rtype;
	struct VarSignature *next_signature;*/
} VarSignature;

typedef struct FunctionSignature {

	u32 type_hash;
	u32 name_hash;
	u8 *basename;
	u8 *type_name;
	u8 *human_name;
	int tok;
	VarSignature return_type;
	VarSignature params[CAPY_MAX_PARAMS];
	u8 param_count;
	struct FunctionSignature* next;

} FunctionSignature;

typedef struct FunctionVariantBucket {

	FunctionSignature* variants;

} FunctionVariantBucket;

typedef struct CallSite {
	char* funcname;
	int funcname_tok;
	Sym* fsym;
	b8 is_capy_func : 1;
	FunctionVariantBucket* bucket;
	Sym *s;
	SValue* vtop_placeholder;
	SValue* param_svalues[CAPY_MAX_PARAMS];
	LinkedList* candidates;
	int param_count;
} CallSite;

LinkedList* capy_list_add(LinkedList* parent) {
	LinkedList* result;
	result = mem_calloc(1, sizeof(LinkedList));
	if(parent)
		parent->next = result;
	return result;
}

void capy_list_remove(LinkedList* element, LinkedList* previous) {
	if(previous)
		previous->next = element->next;
	// tcc_free(element); ?
}

void capy_type_comp_debug(CType* found, CType* expected, int ctr)
{
	char tbuf1[256];
	char tbuf2[256];
	type_to_str(tbuf1, sizeof(tbuf1), expected, NULL);
	type_to_str(tbuf2, sizeof(tbuf2), found, NULL);
	// printf(" param%i[need:%s have:%s] \n", ctr, tbuf1, tbuf2);
}

int capy_type_comp_check(CType* found, FunctionSignature* fnsig, u8 param_index)
{
	//char tbuf1[256];
	int result;
	result = is_type_compatible(found, fnsig->params[param_index].ctype);
	/*type_to_str(tbuf1, sizeof(tbuf1), found, NULL);
	printf(" - checking against %s<%s> #%i:%s to %s => %i\n",
		fnsig->basename, fnsig->human_name, param_index,
		fnsig->params[param_index].type_name,
		tbuf1,
		result);*/
	return(result);
}

bool capy_is_capy_func(const char* s)
{
	FunctionVariantBucket *result;
	if (!capy_signatures)
		return false;
	result = hashmap_get(capy_signatures, (u32)char_hash((char*)s));
	//if(result)
	//	printf(" #%x > %p ", (u32)char_hash((char*)s), result);
	return result != 0;
}

void capy_callsite_addparam(CallSite* cs, SValue* found_type_sv, CType* expected_type)
{
	LinkedList *lsig, *lprev;
	if(cs->param_count <= CAPY_MAX_PARAMS)
		cs->param_svalues[cs->param_count++] = found_type_sv;
	else
		tcc_error("max number of parameters (%i) exceeded", CAPY_MAX_PARAMS);
	capy_type_comp_debug(&found_type_sv->type, expected_type, cs->param_count);
	lsig = cs->candidates;
	lprev = 0;
	while(lsig) {
		int compat;
		FunctionSignature* fnsig = (FunctionSignature*)lsig->val.ptr;
		compat = capy_type_comp_check(&found_type_sv->type, fnsig, cs->param_count-1);
		if(compat <= 0) {
			if(cs->candidates == lsig)
				cs->candidates = lsig->next;
			else if(lprev)
				lprev->next = lsig->next;
			// printf(" (candidate removed) ");
		} else {
			lprev = lsig;
		}
		lsig = lsig->next;
	}
	if(!cs->candidates)
		tcc_error("no matching candidate found for %s()", cs->funcname);
}

bool capy_callsite_namelookup(CallSite* cs)
{
	FunctionVariantBucket *result;
	if (!capy_signatures)
		return false;
	result = hashmap_get(capy_signatures, (u32)char_hash((char*)cs->funcname));
	cs->is_capy_func = result != 0;
	if(cs->is_capy_func)
	{
		int cctr = 0;
		LinkedList* l = 0;
		FunctionSignature* fnsig;
		cs->bucket = result;
		fnsig = cs->bucket->variants;
		while(fnsig) {
			cctr++;
			l = capy_list_add(l);
			if(!cs->candidates)
				cs->candidates = l;
			l->val.ptr = fnsig;
			fnsig = fnsig->next;
		}
		printf(" capy_callsite_namelookup: %s() found, candidates: %i \n", cs->funcname, cctr);
		return true;
	}
	return false;
}

FunctionVariantBucket *capy_register_fn_signature(FunctionSignature *fnsig) {
	FunctionVariantBucket *result;
	if (!capy_signatures) {
		capy_signatures = hashmap_create();
	}
	result = hashmap_get(capy_signatures, fnsig->name_hash);
	if (!result) {
		result = mem_calloc(1, sizeof(FunctionVariantBucket));
		hashmap_insert(capy_signatures, fnsig->name_hash, result);
	}
	if (!result->variants)
		result->variants = fnsig;
	else {
		fnsig->next = result->variants;
		result->variants = fnsig;
	}
	//printf("Register %x signature '%s()' variant:%x %s\n",
	//	fnsig->name_hash, fnsig->basename, fnsig->type_hash, fnsig->human_name);
	return result;
}

void debug_signature(VarSignature *v, char *indent) {
	char *basename;
	char *type_name;
	if (!v)
		return;
	basename = "-";
	type_name = "-";
	if (v->type_name)
		type_name = v->type_name;
	if (indent)
		printf("%s", indent);
	else
		printf("SIGNATURE ");
	printf("hash=%x name=%s btype=%s size=%i asize=%i "
		   "basic:%b func:%b s:%b float:%b bool:%b enum:%b "
		   "ptr:%b struct:%b array:%b const:%b\n",
		   v->type_hash, basename, type_name, v->size, v->array_size,
		   v->is_basic_type, v->is_func, v->is_signed, v->is_float, v->is_bool,
		   v->is_enum, v->is_pointer, v->is_struct, v->is_array, v->is_const);
	//if (v->rtype)
	//	debug_signature(v->rtype, "\t < ");
	//if (v->next_param)
	//	debug_signature(v->next_param, "\t > ");
}

void capy_type_to_signature(VarSignature* result, Sym *sv, char *name, int hash_flag,
									 char *human_name, int tk) {
	int bt, v, t;
	Sym *s;
	CType *type = &sv->type;

	result->s = sv;

cap_sig:
	t = type->t;
	bt = t & VT_BTYPE;

	if (name) {
		//result->basename = capy_create_identifier(name);
		result->name_hash = char_hash(name);
	}
	result->type_hash = hash_flag;
	result->is_signed = !(t & VT_UNSIGNED);
	if(!result->ctype)
		result->ctype = type;
	result->tok = tk;
	if (human_name)
		result->human_name = capy_create_identifier(human_name);

	switch (bt) {
	case VT_VOID:
		result->is_basic_type = 1;
		result->is_void = 1;
		result->size = 64;
		result->type_name = capy_create_identifier("void");
		goto add_tstr;
	case VT_BOOL:
		result->is_basic_type = 1;
		result->is_bool = 1;
		result->size = 32;
		result->type_name = capy_create_identifier("bool");
		goto add_tstr;
	case VT_BYTE:
		result->is_basic_type = 1;
		result->size = 8;
		result->type_name =
			capy_create_identifier(result->is_signed ? "s8" : "u8");
		goto add_tstr;
	case VT_SHORT:
		result->is_basic_type = 1;
		result->size = 16;
		result->type_name =
			capy_create_identifier(result->is_signed ? "s16" : "u16");
		goto add_tstr;
	case VT_INT:
		result->is_basic_type = 1;
		result->size = 32;
		result->type_name =
			capy_create_identifier(result->is_signed ? "s32" : "u32");
		goto maybe_long;
	case VT_LLONG:
		result->size = 128;
		result->type_name =
			capy_create_identifier(result->is_signed ? "s128" : "u128");
	maybe_long:
		result->is_basic_type = 1;
		if (t & VT_LONG) {
			result->type_name =
				capy_create_identifier(result->is_signed ? "s64" : "u64");
			result->size = 64;
		}
		if (!IS_ENUM(t))
			goto add_tstr;
		result->is_enum = 1;
		goto tstruct;
	case VT_FLOAT:
		result->is_basic_type = 1;
		result->is_float = 1;
		result->size = 32;
		result->type_name = capy_create_identifier("f32");
		goto add_tstr;
	case VT_DOUBLE:
		result->is_basic_type = 1;
		result->is_float = 1;
		result->size = 64;
		result->type_name = capy_create_identifier("f64");
		if (!(t & VT_LONG))
			goto add_tstr;
	case VT_LDOUBLE:
		result->is_basic_type = 1;
		result->is_float = 1;
		result->size = 128;
		result->type_name = capy_create_identifier("f128");
	add_tstr:
		// pstrcat(buf, buf_size, tstr);
		break;
	case VT_STRUCT:
		/*
	tstr = "struct ";
	if (IS_UNION(t))
	tstr = "union ";
	*/
		// tstr = "";
	tstruct:
		// pstrcat(buf, buf_size, tstr);
		v = type->ref->v & ~SYM_STRUCT;
		result->type_name =
			capy_create_identifier((char *)get_tok_str(v, NULL));
		result->is_struct = 1;
		break;
	case VT_FUNC: {
		/*
		VarSignature *param;
		result->is_func = 1;

		s = type->ref;

		sa = s->next;
		param = result;
		while (sa != NULL) {
			param->next_param = capy_type_to_signature(&sa->type, "", 0, 0, 0);
			param = param->next_param;
			sa = sa->next;
		}

		result->rtype = capy_type_to_signature(&s->type, "", 0, 0, 0);
		goto no_var;
		*/
		tcc_error("internal error: unexpected VT_FUNC");
	}
	case VT_PTR:
		s = type->ref;
		result->is_pointer = 1;
		if (t & VT_ARRAY) {
			result->is_array = 1;
			result->array_size = s->c;
			if (name && '*' == *name) {
				result->type_name = capy_create_identifier(name + 1);
			} else {
				result->type_name = capy_create_identifier(name);
			}
			goto no_var;
		}
		if (t & VT_CONSTANT)
			result->is_const = 1;
		if (t & VT_VOLATILE)
			result->is_volatile = 1;
		type = &s->type;
		goto cap_sig;
	}
no_var:;
	//return result;
}

FunctionSignature* capy_create_fn_signature(CType *type, char *name, int hash_flag,
									 char *human_name, int tk)
{
	FunctionSignature* result;
	Sym *s, *sa;

	result = mem_calloc(1, sizeof(FunctionSignature));

	if (name) {
		result->basename = capy_create_identifier(name);
		result->name_hash = char_hash(name);
	}
	result->type_hash = hash_flag;
	result->tok = tk;
	if (human_name)
		result->human_name = capy_create_identifier(human_name);

	s = type->ref;
	sa = s->next;
	while (sa != NULL) {
		capy_type_to_signature(&result->params[result->param_count++], sa, "", 0, 0, 0);
		sa = sa->next;
	}
	capy_type_to_signature(&result->return_type, s, "", 0, 0, 0);

	return result;
}

