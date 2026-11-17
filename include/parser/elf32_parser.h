#ifndef ELF32_PARSER_H
#define ELF32_PARSER_H

#include <cstring>
#include <filesystem>
#include <optional>
#include <span>
#include <string_view>
#include <vector>

#if __has_include(<elf.h>)
#include <elf.h>
#else
#define ELF_H
struct Elf32_Ehdr;
struct Elf32_Shdr;
struct Elf32_Phdr;
#endif // __has_include(<elf.h>)



class Elf32Parser {
public:
  using size_type = std::vector<std::byte>::size_type;

  explicit Elf32Parser(const std::filesystem::path& filepath);

  [[nodiscard]] 
  auto getSection(std::string_view sec_name) const
    -> std::optional<std::span<const std::byte>>;
 
  [[nodiscard]] 
  auto getSegment(std::string_view seg_name) const
    -> std::optional<std::span<const std::byte>>;
  
  [[nodiscard]] 
  auto getTextRegion() const
    -> std::optional<std::span<const std::byte>>;

private:
  std::vector<std::byte> buffer_;

  template <typename T> 
  requires std::is_trivially_copyable_v<T>
  [[nodiscard]] 
  T readAt(size_t offset) const;
  
  [[nodiscard]] 
  auto slice(size_t offset, size_t count) const
    -> std::span<const std::byte>;
  
  void validateHeader() const;
};


#ifdef ELF_H
struct Elf32_Ehdr {
  unsigned char e_ident[16];  /* Magic number and other info */
  uint16_t e_type;            /* Object file type */
  uint16_t e_machine;         /* Architecture */
  uint32_t e_version;         /* Object file version */
  uint32_t e_entry;           /* Entry point virtual address */
  uint32_t e_phoff;           /* Program header table file offset */
  uint32_t e_shoff;           /* Section header table file offset */
  uint32_t e_flags;           /* Processor-specific flags */
  uint16_t e_ehsize;          /* ELF header size in bytes */
  uint16_t e_phentsize;       /* Program header table entry size */
  uint16_t e_shentsize;       /* Section header table entry size */
  uint16_t e_shnum;           /* Section header table entry count */
  uint16_t e_shstrndx;        /* Section header string table index */
};

struct Elf32_Shdr {
  uint32_t sh_name;     /* Section name (string tbl index) */
  uint32_t sh_type;     /* Section type */
  uint32_t sh_flags;    /* Section flags */
  uint32_t sh_addr;     /* Section virtual addr at execution */
  uint32_t sh_offset;   /* Section file offset */
  uint32_t sh_size;     /* Section size in bytes */
  uint32_t sh_link;     /* Link to another section */
  uint32_t sh_info;     /* Additional section information */
  uint32_t sh_addralign;  /* Section alignment */
  uint32_t sh_entsize;  /* Entry size if section holds table */
};

struct Elf32_Phdr {
  uint32_t p_type;    	/* Segment type */
  uint32_t p_offset;   	/* Segment file offset */
  uint32_t p_vaddr;     /* Segment virtual address */
  uint32_t p_paddr;     /* Segment physical address */
  uint32_t p_filesz;    /* Segment size in file */
  uint32_t p_memsz;     /* Segment size in memory */
  uint32_t p_flags;     /* Segment flags */
  uint32_t p_align;     /* Segment alignment */
};

#define	PT_NULL		0		  /* Program header table entry unused */
#define PT_LOAD		1		  /* Loadable program segment */
#define PT_DYNAMIC	2		/* Dynamic linking information */
#define PT_INTERP	3		  /* Program interpreter */
#define PT_NOTE		4		  /* Auxiliary information */
#define PT_SHLIB	5		  /* Reserved */
#define PT_PHDR		6		  /* Entry for header table itself */
#define PT_TLS		7		  /* Thread-local storage segment */
#define	PT_NUM		8		  /* Number of defined types */

#define PF_X		(1 << 0)	/* Segment is executable */
#define PF_W		(1 << 1)	/* Segment is writable */
#define PF_R		(1 << 2)	/* Segment is readable */
#endif // ELF_H

#endif // ELF32_PARSER_H
