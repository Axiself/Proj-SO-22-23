#include "fs/operations.h"
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

uint8_t const ext_file[] = "ABC. DEF: GHI( JKL) MNO! PQR? STU, VWX; YZ- ";
char const path_ext_file[] = "tests/ex1_small.txt";
char const target_path1[] = "/f1";
char const link_path1[] = "/l1";
char const link_path2[] = "/l2";
char const link_path3[] = "/l3";
char const link_path4[] = "/l4";
char const invalid_path[] = "/";

void assert_contents_ok(char const *path) {
    printf("ok - %s\n", path);

    int f = tfs_open(path, 0);
    assert(f != -1);

    uint8_t buffer[sizeof(ext_file)];
    assert(tfs_read(f, buffer, sizeof(buffer)) == sizeof(buffer));
    assert(memcmp(buffer, ext_file, sizeof(buffer)) == 0);

    assert(tfs_close(f) != -1);

    printf("end check\n");
}

void assert_empty_file(char const *path) {
    printf("empty - %s\n", path);

    int f = tfs_open(path, 0);
    assert(f != -1);

    uint8_t buffer[sizeof(ext_file)];
    assert(tfs_read(f, buffer, sizeof(buffer)) == 0);

    assert(tfs_close(f) != -1);

    printf("end check\n");
}

void write_contents(char const *path) {
    int f = tfs_open(path, 0);
    assert(f != -1);

    assert(tfs_write(f, ext_file, sizeof(ext_file)) ==
           sizeof(ext_file));

    assert(tfs_close(f) != -1);
}

int main() {
    int fh;
    assert(tfs_init(NULL) != -1);

    // Write to new hard link and read original file
    {
        int f = tfs_open(target_path1, TFS_O_CREAT);
        assert(f != -1);
        assert(tfs_close(f) != -1);

        assert_empty_file(target_path1); // sanity check
    }

    assert(tfs_link(target_path1, link_path1) != -1);
    assert_empty_file(link_path1);

    write_contents(link_path1);
    assert_contents_ok(target_path1);

    {
        int f = tfs_open(link_path1, 0);
        assert(f != -1);
        assert(tfs_close(f) != -1);
        assert_contents_ok(link_path1); // sanity check
    }

    // Create soft link to hard link
    assert(tfs_sym_link(link_path1, link_path2) != -1);
    {
        int f = tfs_open(link_path2, 0);
        assert(f != -1);
        assert(tfs_close(f) != -1);

        assert_contents_ok(link_path2); // sanity check
    }

    // Attempt to create hard link to soft link
    assert(tfs_link(link_path2, link_path3) == -1);

    // Attempt to create links with invalid paths
    assert(tfs_link(invalid_path, link_path4) == -1);
    assert(tfs_sym_link(invalid_path, link_path4) == -1);

    // Delete file then links in order and check validity

    // Delete file
    assert(tfs_unlink(target_path1) != -1);

    fh = tfs_open(link_path1, TFS_O_APPEND);
    assert(fh != -1);
    tfs_close(fh);

    fh = tfs_open(link_path2, TFS_O_APPEND);
    assert(fh != -1);
    tfs_close(fh);

    // Delete hard link
    assert(tfs_unlink(link_path1) != -1);
    assert(tfs_open(link_path1, TFS_O_APPEND) == -1);
    assert(tfs_open(link_path2, TFS_O_APPEND) == -1);

    // Create hard link through soft link
    assert(tfs_open(link_path2, TFS_O_CREAT) != -1);
    assert(tfs_open(link_path1, TFS_O_APPEND) != -1);

    assert(tfs_destroy() != -1);

    printf("Successful test.\n");

    return 0;
}