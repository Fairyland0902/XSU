#ifndef _XSU_FS_FAT_H
#define _XSU_FS_FAT_H

#include <xsu/fs/fscache.h>
#include <xsu/type.h>

// 4k data buffer number in each file struct.
#define LOCAL_DATA_BUF_NUM 4

#define SECTOR_SIZE 512
#define CLUSTER_SIZE 4096

struct __attribute__((__packed__)) dir_entry_attr {
    u8 name[8]; // Name.
    u8 ext[3]; // Extension.
    u8 attr; // Attribute bits.
    u8 lcase; // Case for base and extension.
    u8 ctime_cs; // Creation time, centiseconds(0 - 199).
    u16 ctime; // Creation time.
    u16 cdate; // Creation date.
    u16 adate; // Last access date.
    u16 starthi; // Start cluster (high 16 bits).
    u16 time; // Last modify time.
    u16 date; // Last modify date.
    u16 startlow; // Start cluster (low 16 bits).
    u32 size; // File size (in bytes).
};

union dir_entry {
    u8 data[32];
    struct dir_entry_attr attr;
};

// File struct.
typedef struct fat_file {
    u8 path[256];
    // Current file pointer.
    u32 loc;
    // Current directory entry position.
    u32 dir_entry_pos;
    u32 dir_entry_sector;
    // Current directory entry.
    union dir_entry entry;
    // Buffer clock head.
    u32 clock_head;
    // For normal FAT32, cluster size is 4k.
    BUF_4K data_buf[LOCAL_DATA_BUF_NUM];
} FILE;

typedef struct fs_fat_dir {
    u32 cur_sector;
    u32 loc;
    u32 sec;
} FS_FAT_DIR;

struct __attribute__((__packed__)) BPB_attr {
    // 0x00 ~ 0x0f
    u8 jump_code[3];
    u8 oem_name[8];
    u16 sector_size;
    u8 sectors_per_cluster;
    u16 reserved_sectors;
    // 0x10 ~ 0x1f
    u8 number_of_copies_of_fat;
    u16 max_root_dir_entries;
    u16 num_of_small_sectors;
    u8 media_descriptor;
    u16 sectors_per_fat;
    u16 sectors_per_track;
    u16 num_of_heads;
    u32 num_of_hidden_sectors;
    // 0x20 ~ 0x2f
    u32 num_of_sectors;
    u32 num_of_sectors_per_fat;
    u16 flags;
    u16 version;
    u32 cluster_number_of_root_dir;
    // 0x30 ~ 0x3f
    u16 sector_number_of_fs_info;
    u16 sector_number_of_backup_boot;
    u8 reserved_data[12];
    // 0x40 ~ 0x51
    u8 logical_drive_number;
    u8 unused;
    u8 extended_signature;
    u32 serial_number;
    u8 volume_name[11];
    // 0x52 ~ 0x1fe
    u8 fat_name[8];
    u8 exec_code[420];
    u8 boot_record_signature[2];
};

union BPB_info {
    u8 data[512];
    struct BPB_attr attr;
}

struct fs_info {
    u32 base_addr;
    u32 sectors_per_fat;
    u32 total_sectors;
    u32 total_data_clusters;
    u32 total_data_sectors;
    u32 first_data_sector;
    union BPB_info BPB;
    u8 fat_fs_info[SECTOR_SIZE];
};

u32 fs_find(FILE* file);
u32 init_fs();
u32 fs_open(FILE* file, u8* filename);
u32 fs_close(FILE* file);
u32 fs_read(FILE* file, u8* buf, u32 count);
u32 fs_write(FILE* file, const u8* buf, u32 count);
u32 fs_fflush();
void fs_lseek(FILE* file, u32 new_loc);
u32 fs_create(u8* filename);
u32 fs_mkdir(u8* filename);
u32 fs_rm(u8* filename);
u32 fs_mv(u8* src, u8* dest);
u32 fs_open_dir(FS_FAT_DIR* dir, u8* filename);
u32 fs_read_dir(FS_FAT_DIR* dir, u8* buf);
u32 fs_cat(u8* path);
void get_filename(u8* entry, u8* buf);
u32 read_block(u8* buf, u32 addr, u32 count);
u32 write_block(u8* buf, u32 addr, u32 count);
u32 get_entry_filesize(u8* entry);
u32 get_entry_attr(u8* entry);

#endif
