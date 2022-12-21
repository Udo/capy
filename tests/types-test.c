void run_types_tests()
{
	struct string* s1 = string_create();
	struct string* s2;
	struct string* s3;
	struct string* s4;
	string_assign_chars(s1, " - this will probably work");

#ifdef TEST_SUBSTR
	printf("Test: string assign = '%s' length=%i \n", s1->bytes, s1->length);

	for(s32 i = -s1->length; i < s1->length+6; i += 4)
	{
		s2 = string_substr(s1, 0, i);
		printf("Test: string_substr(0, %i) = '%s' \n", i, s2->bytes);
		string_free(s2);
	}
	if(0)
	for(s32 i = -s1->length; i < s1->length+6; i += 4)
	{
		s2 = string_substr(s1, i, 0);
		printf("Test: string_substr(%i, 0) = '%s' \n", i, s2->bytes);
		string_free(s2);
	}
#endif

#ifdef TEST_POS
	s2 = string_create_with_chars("this");
	printf("Test: string_pos('%s') = %i \n", s2->bytes, string_pos(s1, s2, 0));
	string_free(s2);
	s2 = string_create_with_chars("will");
	printf("Test: string_pos('%s') = %i \n", s2->bytes, string_pos(s1, s2, 0));
	string_free(s2);
	s2 = string_create_with_chars("rob");
	printf("Test: string_pos('%s') = %i \n", s2->bytes, string_pos(s1, s2, 0));
	string_free(s2);
	s2 = string_create_with_chars("i");
	printf("Test: string_pos('%s') = %i \n", s2->bytes, string_pos(s1, s2, 0));
	string_free(s2);
	s2 = string_create_with_chars("i");
	printf("Test: string_pos('%s', 5) = %i \n", s2->bytes, string_pos(s1, s2, 5));
	string_free(s2);
	s2 = string_create_with_chars("k");
	printf("Test: string_pos('%s') = %i \n", s2->bytes, string_pos(s1, s2, 0));
	string_free(s2);
	s2 = string_create_with_chars("ix");
	printf("Test: string_pos('%s') = %i \n", s2->bytes, string_pos(s1, s2, 0));
	string_free(s2);
	s2 = string_create_with_chars("");
	printf("Test: string_pos('%s') = %i \n", s2->bytes, string_pos(s1, s2, 0));
	string_free(s2);
	s2 = string_create_with_chars("this will probably work!");
	printf("Test: string_pos('%s') = %i \n", s2->bytes, string_pos(s1, s2, 0));
	string_free(s2);
#endif

#ifdef TEST_CONTAINS
	s2 = string_create_with_chars("will");
	printf("Test: string_contains('%s') = %i \n", s2->bytes, string_contains(s1, s2, 0));
	string_free(s2);
	s2 = string_create_with_chars("will not");
	printf("Test: string_contains('%s') = %i \n", s2->bytes, string_contains(s1, s2, 0));
	string_free(s2);
#endif

#ifdef TEST_TRIM
	s3 = string_create_with_chars("   this is a string with some stuff around it \t\n  ");
	s2 = string_trim(s3);
	printf("Test: string_trim() = '%s'\n", s2->bytes);
	string_free(s3);
	string_free(s2);
	s3 = string_create_with_chars("     \t\n  ");
	s2 = string_trim(s3);
	printf("Test: string_trim() = '%s'\n", s2->bytes);
	string_free(s3);
	string_free(s2);
	s3 = string_create_with_chars("   X  \t\n  ");
	s2 = string_trim(s3);
	printf("Test: string_trim() = '%s'\n", s2->bytes);
	string_free(s3);
	string_free(s2);
#endif

#ifdef TEST_CHAR_AT
	printf("Test: string_char_at(3) = %c\n", string_char_at(s1, 3));
	printf("Test: string_char_at(-3) = %c\n", string_char_at(s1, -3));
	printf("Test: string_char_at(0) = %c\n", string_char_at(s1, 0));
#endif

#ifdef TEST_LOWER_UPPER
	s2 = string_to_upper(s1);
	s3 = string_to_lower(s2);
	printf("Test: string_to_upper() = %s\n", s2->bytes);
	printf("Test: string_to_lower() = %s\n", s3->bytes);
	string_free(s2);
	string_free(s3);
#endif

#ifdef TEST_NIBBLE
	s2 = string_create();
	s4 = string_create_with_chars(" ");
	string_assign_bytes(s2, s1->bytes, s1->length);
	do {

		s3 = string_nibble(s2, s4);
		printf("Test: string_nibble('%s') = '%s' | '%s'\n", s4->bytes, s3->bytes, s2->bytes);

	} while(s2->length > 0);
	string_free(s2);
#endif

#ifdef TEST_COMPARE
	s2 = string_create_with_chars("StringA");
	s3 = string_create_with_chars("StringB");
	printf("Test: string_compare('%s', '%s') = %i\n", s2->bytes, s3->bytes, string_compare(s2, s3));
	string_free(s2);
	string_free(s3);
	s2 = string_create_with_chars("StringB");
	s3 = string_create_with_chars("StringA");
	printf("Test: string_compare('%s', '%s') = %i\n", s2->bytes, s3->bytes, string_compare(s2, s3));
	string_free(s2);
	string_free(s3);
	s2 = string_create_with_chars("StringA");
	s3 = string_create_with_chars("StringA");
	printf("Test: string_compare('%s', '%s') = %i\n", s2->bytes, s3->bytes, string_compare(s2, s3));
	string_free(s2);
	string_free(s3);
	s2 = string_create_with_chars("StringAAAAAAAAAA");
	s3 = string_create_with_chars("StringA");
	printf("Test: string_compare('%s', '%s') = %i\n", s2->bytes, s3->bytes, string_compare(s2, s3));
	string_free(s2);
	string_free(s3);
#endif

#ifdef TEST_REPLACE
	s2 = string_create_with_chars("wi");
	s3 = string_create_with_chars("WI");
	s4 = string_replace(s1, s2, s3);
	printf("Test: string_replace('%s', '%s', '%s') = '%s'\n", s1->bytes, s2->bytes, s3->bytes, s4->bytes);
	string_free(s2);
	string_free(s3);
	string_free(s4);
	s2 = string_create_with_chars("wi");
	s3 = string_create_with_chars("");
	s4 = string_replace(s1, s2, s3);
	printf("Test: string_replace('%s', '%s', '%s') = '%s'\n", s1->bytes, s2->bytes, s3->bytes, s4->bytes);
	string_free(s2);
	string_free(s3);
	string_free(s4);
	s2 = string_create_with_chars("wi123");
	s3 = string_create_with_chars("NOPE");
	s4 = string_replace(s1, s2, s3);
	printf("Test: string_replace('%s', '%s', '%s') = '%s'\n", s1->bytes, s2->bytes, s3->bytes, s4->bytes);
	string_free(s2);
	string_free(s3);
	string_free(s4);
#endif

#ifdef TEST_FILE_GET_PUT
	s2 = string_create_with_chars("/etc/crontab");
	s3 = file_get_contents(s2);
	printf("Test: file read from %s: %i bytes\n", s2->bytes, s3->length);
	string_free(s2);

	s2 = string_create_with_chars("/tmp/capy_test");
	printf("Test: file write to %s: %b\n", s2->bytes, file_put_contents(s2, s3));
	string_free(s2);
#endif

#ifdef TEST_LIST
	struct list* l1 = list_create();
	list_append(l1, string_create_with_chars("list entry #1"));
	list_append(l1, string_create_with_chars("list entry #2"));
	list_append(l1, string_create_with_chars("list entry #3"));
	list_append(l1, string_create_with_chars("list entry #4"));
	list_append(l1, string_create_with_chars("list entry #5"));
	list_append(l1, string_create_with_chars("list entry #6"));
	list_append(l1, string_create_with_chars("list entry #7"));
	list_append(l1, string_create_with_chars("list entry #8"));
	printf("Test: list length %i | first %p\n", l1->length, l1->first);
	for(struct list_item* si = l1->first; si != 0; si = si->next)
	{
		printf("Test: list_item() %s\n", ((struct string*)si->data)->bytes);
	}
	s2 = list_delete_item(l1, l1->last);
	printf("Test: list_delete_item(last) %s, list size now %i, first %p, last %p\n", s2->bytes, l1->length, l1->first, l1->last);
	s2 = list_delete_item(l1, l1->first);
	printf("Test: list_delete_item(first) %s, list size now %i, first %p, last %p\n", s2->bytes, l1->length, l1->first, l1->last);
	s2 = list_pop(l1);
	printf("Test: list_pop() %s, list size now %i, first %p, last %p\n", s2->bytes, l1->length, l1->first, l1->last);
	s2 = list_shift(l1);
	printf("Test: list_shift() %s, list size now %i, first %p, last %p\n", s2->bytes, l1->length, l1->first, l1->last);

	list_insert_after(l1, l1->first, string_create_with_chars("list entry gen#2 #1"));
	printf("Test: list_insert_after(first), list size now %i, first %p, last %p\n", l1->length, l1->first, l1->last);
	list_insert_after(l1, l1->last, string_create_with_chars("list entry gen#2 #2"));
	printf("Test: list_insert_after(last), list size now %i, first %p, last %p\n", l1->length, l1->first, l1->last);
	list_prepend(l1, string_create_with_chars("list entry gen#2 #3"));
	printf("Test: list_prepend(), list size now %i, first %p, last %p\n", l1->length, l1->first, l1->last);
	for(struct list_item* si = l1->first; si != 0; si = si->next)
	{
		printf("Test: list_item() %s %p\n", ((struct string*)si->data)->bytes, string_hash(si->data));
	}
	printf("And now reverse:\n");
	for(struct list_item* si = l1->last; si != 0; si = si->prev)
	{
		printf("Test: list_item() %s %p\n", ((struct string*)si->data)->bytes, string_hash(si->data));
	}
#endif

#define TEST_HASHMAP
#ifdef TEST_HASHMAP
	struct hashmap* h1 = hashmap_create();
	hashmap_insert_with_char_key(h1, "key1", string_create_with_chars("this is the content of key1"));
	hashmap_insert_with_char_key(h1, "key2", string_create_with_chars("this is the content of key2"));
	hashmap_insert_with_char_key(h1, "key3", string_create_with_chars("this is the content of key3"));
	hashmap_insert_with_char_key(h1, "key4", string_create_with_chars("this is the content of key4"));
	hashmap_insert_with_char_key(h1, "key5", string_create_with_chars("this is the content of key5"));
	hashmap_insert_with_char_key(h1, "key6", string_create_with_chars("this is the content of key6"));
	hashmap_insert_with_char_key(h1, "key7", string_create_with_chars("this is the content of key7"));
	hashmap_insert_with_char_key(h1, "key8", string_create_with_chars("this is the content of key8"));
	hashmap_insert_with_char_key(h1, "key9", string_create_with_chars("this is the content of key9"));
	hashmap_insert_with_char_key(h1, "key10", string_create_with_chars("this is the content of key10"));
	printf("Hashmap delete item data pointer %p\n", hashmap_delete_with_char_key(h1, "key3"));
	printf("Hashmap delete item data pointer %p\n", hashmap_delete_with_char_key(h1, "key312"));
	struct hashmap_item* it = hashmap_first_entry(h1);
	while(it)
	{
		printf("HashMap content hash=%p bucket=%i key=%s :: %s\n", it->hash, it->bucket_idx, it->key->bytes, ((struct string*)it->data)->bytes);
		it = hashmap_next_entry(h1, it);
	}
#endif

#ifdef TEST_SPLIT
	struct list* il = string_split(string_create_with_chars("a,bc,de,,,fghi"), string_create_with_chars(","));
	struct list_item* ili = il->first;
	while(ili)
	{
		printf("Split item: %s\n", ((struct string*)ili->data)->bytes);
		ili = ili->next;
	}
#endif

}
