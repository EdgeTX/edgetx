.ccm (NOLOAD) :
{
  . = ALIGN(4);
  _sccm = .;
  *(.ccm)
  . = ALIGN(4);
  _eccm = .;
} > CCM
