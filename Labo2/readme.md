# Laboratorio 2

## Integrantes
- Carlos Miguel Espinoza Peralta : <carlos.espinoza24@unmsm.edu.pe>
- Sebastian Rojas Rojas : <sebastian.rojas6@unmsm.edu.pe>
- Abraham Carbajal Gutierres : <abraham.carbajal2@unmsm.edu.pe>

## Syscall Write

### 1. Thread Structure

```c

struct thread
  {
    ...
    struct list files;                  
    int max_file_fd;                    

    #ifdef USERPROG
        uint32_t *pagedir;                  
    #endif

    /* Owned by thread.c. */
    unsigned magic;                     
  };

```

Para gestionar los archivos de un proceso se manejo una lista enlazada de archivos abiertos por el proceso, ademas de un entero que lleva la cuenta de los archivos abiertos por el proceso.

```c

struct thread_file
{
    int fd;  // a file descriptor
    struct file* file;
    struct list_elem file_elem;
};

```

Se creo una estructura para manejar los archivos abiertos por el proceso, esta estructura contiene un file descriptor, un puntero al archivo y un puntero a la lista de archivos del proceso.

### Algoritmo

1. Para validar la operacion se debe validar los punteros, si estos son validos se procede a la operacion, en caso contrario se termina el proceso.
```c
void *
check_ptr2(const void *vaddr)
{
  if (!is_user_vaddr(vaddr))
  {
    thread_exit();
  }
  void *ptr = pagedir_get_page(thread_current()->pagedir, vaddr);
  if (!ptr)
  {
    thread_exit();
  }
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
```
2. Una vez que se validan los campos se evoca a la creacion de un thread_file, el cual se encargara de gestionar el archivo abierto por el proceso. Para lo cual usamos lock y asi evitar problemas de concurrencia.

```c
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
```
3. Se recorre la lista de archivos abiertos por el proceso y se compara el file descriptor con el file descriptor pasado por parametro, si se encuentra el archivo se retorna el thread_file, en caso contrario se retorna false.
```c
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

```
1. Se creo una funcion para encontrar el archivo abierto por el proceso, esta funcion recorre la lista de archivos abiertos por el proceso y compara el file descriptor con el file descriptor pasado por parametro.



## Discusion

### Concurrencia
Para evitar problemas de concurrencia se uso lock, el cual se encarga de bloquear el acceso a la lista de archivos abiertos por el proceso, de esta manera se evita que dos hilos accedan a la lista al mismo tiempo.

### Validacion de punteros
Para validar los punteros se uso la funcion check_ptr2, la cual se encarga de verificar que el puntero sea valido y que se encuentre en el espacio de memoria correcto, ademas de verificar que los 4 bytes siguientes al puntero sean accesibles.

### Manejo de archivos
Para manejar los archivos se uso una lista enlazada de archivos abiertos por el proceso, ademas de una estructura thread_file que se encarga de gestionar los archivos abiertos por el proceso.





