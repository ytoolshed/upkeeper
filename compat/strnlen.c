size_t
strnlen(const char * restrict s, size_t maxlen)
{
	size_t n = 0;
	while( n <= maxlen && *(s+n++) != '\0');

	return n;
}
