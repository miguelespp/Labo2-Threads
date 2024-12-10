#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "threads/synch.h"
#include "userprog/pagedir.h"

#define max_syscall 21

static struct lock lock_f;
static void syscall_handler(struct intr_frame *);

static void (*syscalls[max_syscall])(struct intr_frame *);

// declaration of syscalls

void sys_write(struct intr_frame *f);

// funtions declarations

void *check_ptr2(const void *vaddr);
struct thread_file *find_file_id(int file_id);

void syscall_init(void)
{
  intr_register_int(0x30, 3, INTR_ON, syscall_handler, "syscall");
  syscalls[SYS_WRITE] = &sys_write;
}

static void
syscall_handler(struct intr_frame *f UNUSED)
{
  int *p = f->esp;
  check_ptr2(p + 1);
  int type = *(int *)f->esp;
  if (type <= 0 || type >= max_syscall)
  {
    thread_exit();
  }
  syscalls[type](f);
}

/* Do system write, Do writing in stdout and write in files */
void sys_write(struct intr_frame *f)
{
  uint32_t *user_ptr = f->esp;
  check_ptr2(user_ptr + 7); // 
  check_ptr2(*(user_ptr + 6));
  *user_ptr++;
  int fd = *user_ptr;
  const char *buffer = (const char *)*(user_ptr + 1);
  off_t size = *(user_ptr + 2);
  if (fd == 1)
  { // writes to the console
    /* Use putbuf to do testing */
    putbuf(buffer, size);
    f->eax = size; 
  }
  else
  {
    /* Write to Files */
    struct thread_file *thread_file_temp = find_file_id(*user_ptr);
    if (thread_file_temp)
    {
      // lock for files
      acquire_lock_f(); 
      f->eax = file_write(thread_file_temp->file, buffer, size);
      release_lock_f();
    }
    else
    {
      f->eax = 0; // can't write,return 0
    }
  }
}

/* Find file by the file's ID */
struct thread_file *
find_file_id(int file_id)
{
  struct list_elem *e;
  struct thread_file *thread_file_temp = NULL;
  struct list *files = &thread_current()->files;
  for (e = list_begin(files); e != list_end(files); e = list_next(e))
  {
    thread_file_temp = list_entry(e, struct thread_file, file_elem);
    if (file_id == thread_file_temp->fd)
      return thread_file_temp;
  }
  return false;
}

void init_lock_f()
{
  lock_init(&lock_f); 
}

void acquire_lock_f()
{
  lock_acquire(&lock_f);
}

void release_lock_f()
{
  lock_release(&lock_f);
}

static int
get_user(const uint8_t *uaddr)
{
  int result;
  asm("movl $1f, %0; movzbl %1, %0; 1:"
      : "=&a"(result) : "m"(*uaddr));
  return result;
}

/* Function to validate the address is own user,
     as well as the address is in the user space
      and the content of the address is valid
      I forgive you for everything ticona
      */
void *
check_ptr2(const void *vaddr)
{
  if (!is_user_vaddr(vaddr))
  {
    thread_exit();
  }
  /* Judge the page */
  void *ptr = pagedir_get_page(thread_current()->pagedir, vaddr);
  if (!ptr)
  {
    thread_exit();
  }
  /* Judge the content page for the address */
  uint8_t *check_byteptr = (uint8_t *)vaddr;
  for (uint8_t i = 0; i < 4; i++)
  {
    if (get_user(check_byteptr + i) == -1)
    {
      thread_exit();
    }
  }

  return ptr;
}
