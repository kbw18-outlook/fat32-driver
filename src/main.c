extern void test_iter1();
extern void print_clu_size();
extern void test_open_dir_root();
extern void test_find_base_from_name();
int main()
{
#ifdef _DEBUG
    test_find_base_from_name();
#endif
    return 0;
}
