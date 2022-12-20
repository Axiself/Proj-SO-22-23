#include "../fs/operations.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main() {

    char *str_large_ext_file =
        "ABC. DEF: GHI( JKL) MNO! PQR? STU, VWX; YZ- "
        "ABC. DEF: GHI( JKL) MNO! PQR? STU, VWX; YZ- "
        "ABC. DEF: GHI( JKL) MNO! PQR? STU, VWX; YZ- "
        "ABC. DEF: GHI( JKL) MNO! PQR? STU, VWX; YZ- "
        "ABC. DEF: GHI( JKL) MNO! PQR? STU, VWX; YZ- "
        "ABC. DEF: GHI( JKL) MNO! PQR? STU, VWX; YZ- "
        "ABC. DEF: GHI( JKL) MNO! PQR? STU, VWX; YZ- "
        "ABC. DEF: GHI( JKL) MNO! PQR? STU, VWX; YZ- "
        "ABC. DEF: GHI( JKL) MNO! PQR? STU, VWX; YZ- "
        "ABC. DEF: GHI( JKL) MNO! PQR? STU, VWX; YZ- "
        "ABC. DEF: GHI( JKL) MNO! PQR? STU, VWX; YZ- "
        "ABC. DEF: GHI( JKL) MNO! PQR? STU, VWX; YZ- "
        "ABC. DEF: GHI( JKL) MNO! PQR? STU, VWX; YZ- "
        "ABC. DEF: GHI( JKL) MNO! PQR? STU, VWX; YZ- ";
    char *path_large_ext_file = "tests/ex1_large.txt";
    char *str_small_ext_file = "ABC. DEF: GHI( JKL) MNO! PQR? STU, VWX; YZ- ";
    char *path_small_ext_file = "tests/ex1_small.txt";
    char *str_empty_ext_file = "";
    char *path_empty_ext_file = "tests/empty_file.txt";
    char buffer[1024];
    char *copied_file = "/f1";
    char *invalid_pathname = "f1";
    char *invalid_src = "/does_not_exist.txt";
    char *path_one_block_file = "tests/ex1_larger1block.txt";
    int f;
    ssize_t r;

    assert(tfs_init(NULL) != -1);
    f = tfs_open(copied_file, TFS_O_CREAT);
    assert(f != -1);

    //Copy large file first
    assert(tfs_copy_from_external_fs(path_large_ext_file, copied_file) != -1);
    r = tfs_read(f, buffer, sizeof(buffer)-1);
    assert(r == strlen(str_large_ext_file));
    assert(!memcmp(buffer, str_large_ext_file, strlen(str_large_ext_file)));

    //Copy smaller file next
    assert(tfs_copy_from_external_fs(path_small_ext_file, copied_file) != -1);
    r = tfs_read(f, buffer, sizeof(buffer)-1);
    assert(r == strlen(str_small_ext_file));
    assert(!memcmp(buffer, str_small_ext_file, strlen(str_small_ext_file)));

    //Lastly copy empty file
    assert(tfs_copy_from_external_fs(path_empty_ext_file, copied_file) != -1);
    r = tfs_read(f, buffer, sizeof(buffer)-1);
    assert(r == strlen(str_empty_ext_file));
    assert(!memcmp(buffer, str_empty_ext_file, strlen(str_empty_ext_file)));

    //Open with invalid path name
    assert(tfs_open(invalid_pathname, TFS_O_CREAT) == -1);

    //Copy from a non-existant file
    assert(tfs_copy_from_external_fs(invalid_src, copied_file) == -1);

    //Copy file bigger than 1 block
    assert(tfs_copy_from_external_fs(path_one_block_file, copied_file) != -1);
    r = tfs_read(f, buffer, sizeof(buffer)-1);
    assert(r == tfs_default_params().block_size);

    return 0;
}