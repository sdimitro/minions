         K   J       �����������C�׫�E�N�oƳc��            uint
_schedfork(Proc *p)
{
	return ffork(RFMEM|RFNOWAIT, _schedinit, p);
}
