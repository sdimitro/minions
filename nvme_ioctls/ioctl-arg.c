#include <stdint.h>
#include <stdio.h>

/* Copied from
 * https://elixir.bootlin.com/linux/v6.12.6/source/include/uapi/asm-generic/ioctl.h */
#define _IOC_TYPECHECK(t) (sizeof(t))
#define _IOC_NONE	0U
#define _IOC_WRITE	1U
#define _IOC_READ	2U

#define _IOC_NRBITS	8
#define _IOC_TYPEBITS	8
#define _IOC_SIZEBITS	14
#define _IOC_DIRBITS	2

#define _IOC_NRSHIFT	0
#define _IOC_TYPESHIFT	(_IOC_NRSHIFT+_IOC_NRBITS)
#define _IOC_SIZESHIFT	(_IOC_TYPESHIFT+_IOC_TYPEBITS)
#define _IOC_DIRSHIFT	(_IOC_SIZESHIFT+_IOC_SIZEBITS)

#define _IOC(dir,type,nr,size) \
	(((dir)  << _IOC_DIRSHIFT) | \
	 ((type) << _IOC_TYPESHIFT) | \
	 ((nr)   << _IOC_NRSHIFT) | \
	 ((size) << _IOC_SIZESHIFT))

#define _IO(type,nr)		_IOC(_IOC_NONE,(type),(nr),0)
#define _IOR(type,nr,size)	_IOC(_IOC_READ,(type),(nr),(_IOC_TYPECHECK(size)))
#define _IOW(type,nr,size)	_IOC(_IOC_WRITE,(type),(nr),(_IOC_TYPECHECK(size)))
#define _IOWR(type,nr,size)	_IOC(_IOC_READ|_IOC_WRITE,(type),(nr),(_IOC_TYPECHECK(size)))

/* Copied from smartmontools/linux_nvme_ioctl.h */
struct nvme_user_io {
        uint8_t    opcode;
        uint8_t    flags;
        uint16_t   control;
        uint16_t   nblocks;
        uint16_t   rsvd;
        uint64_t   metadata;
        uint64_t   addr;
        uint64_t   slba;
        uint32_t   dsmgmt;
        uint32_t   reftag;
        uint16_t   apptag;
        uint16_t   appmask;
};

struct nvme_passthru_cmd {
        uint8_t    opcode;
        uint8_t    flags;
        uint16_t   rsvd1;
        uint32_t   nsid;
        uint32_t   cdw2;
        uint32_t   cdw3;
        uint64_t   metadata;
        uint64_t   addr;
        uint32_t   metadata_len;
        uint32_t   data_len;
        uint32_t   cdw10;
        uint32_t   cdw11;
        uint32_t   cdw12;
        uint32_t   cdw13;
        uint32_t   cdw14;
        uint32_t   cdw15;
        uint32_t   timeout_ms;
        uint32_t   result;
};

#define nvme_admin_cmd nvme_passthru_cmd

#define SPDK_NVMF_TRSTRING_MAX_LEN 32
#define SPDK_NVMF_TRADDR_MAX_LEN 256
struct cuse_transport {
        char trstring[SPDK_NVMF_TRSTRING_MAX_LEN + 1];
        char traddr[SPDK_NVMF_TRADDR_MAX_LEN + 1];
};

/* What I've been wanting to look up */
#define NVME_IOCTL_ID           _IO('N', 0x40)
#define NVME_IOCTL_ADMIN_CMD    _IOWR('N', 0x41, struct nvme_admin_cmd)
#define NVME_IOCTL_SUBMIT_IO    _IOW('N', 0x42, struct nvme_user_io)
#define NVME_IOCTL_IO_CMD       _IOWR('N', 0x43, struct nvme_passthru_cmd)
#define NVME_IOCTL_RESET        _IO('N', 0x44)
#define NVME_IOCTL_SUBSYS_RESET _IO('N', 0x45)
#define NVME_IOCTL_RESCAN       _IO('N', 0x46)

/* also */
#define BLKGETSIZE   _IO(0x12,96)  /* return device size /512 (long *arg) */
#define BLKSSZGET    _IO(0x12,104) /* get block device sector size */
#define BLKPBSZGET   _IO(0x12,123)
#define BLKGETSIZE64 _IOR(0x12,114,size_t) /* return device size in bytes (u64 *arg) */

#define SPDK_CUSE_GET_TRANSPORT _IOWR('n', 0x1, struct cuse_transport)

int
main(void)
{
	printf("NVME_IOCTL_ID: 0x%X | %d\n", NVME_IOCTL_ID, NVME_IOCTL_ID);
	printf("NVME_IOCTL_ADMIN_CMD: 0x%lX | %ld\n", NVME_IOCTL_ADMIN_CMD, NVME_IOCTL_ADMIN_CMD);
	printf("NVME_IOCTL_SUBMIT_IO: 0x%lX | %ld\n", NVME_IOCTL_SUBMIT_IO, NVME_IOCTL_SUBMIT_IO);
	printf("NVME_IOCTL_IO_CMD: 0x%lX | %ld\n", NVME_IOCTL_IO_CMD, NVME_IOCTL_IO_CMD);
	printf("NVME_IOCTL_RESET: 0x%lX | %ld\n", NVME_IOCTL_RESET, NVME_IOCTL_RESET);
	printf("NVME_IOCTL_SUBSYS_RESET: 0x%lX | %ld\n", NVME_IOCTL_SUBSYS_RESET, NVME_IOCTL_SUBSYS_RESET);
	printf("NVME_IOCTL_RESCAN: 0x%lX | %ld\n", NVME_IOCTL_RESCAN, NVME_IOCTL_RESCAN);
	printf("BLKGETSIZE: 0x%lX | %ld\n", BLKGETSIZE, BLKGETSIZE);
	printf("BLKSSZGET: 0x%lX | %ld\n", BLKSSZGET, BLKSSZGET);
	printf("BLKPBSZGET: 0x%lX | %ld\n", BLKPBSZGET, BLKPBSZGET);
	printf("BLKGETSIZE64: 0x%lX | %ld\n", BLKGETSIZE64, BLKGETSIZE64);
	printf("SPDK_CUSE_GET_TRANSPORT: 0x%lX | %ld\n", SPDK_CUSE_GET_TRANSPORT, SPDK_CUSE_GET_TRANSPORT);
	return (0);
}
