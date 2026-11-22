/* Kobo firmware 4.44 ships glibc 2.11.1.  Keep the two ARM REL tables
 * adjacent so an eager relocation pass can never interpret .ARM.exidx as
 * relocation records. */
SECTIONS
{
  .rel.dyn : { *(.rel.dyn) }
  .rel.plt : { *(.rel.plt) }
}
INSERT BEFORE .ARM.exidx;
