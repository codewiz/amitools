from amitools.vamos.libstructs import ProcessStruct, CLIStruct, PathListStruct, NodeType
from amitools.vamos.astructs import AmigaClassDef
from .var import LocalVar


@AmigaClassDef
class PathList(PathListStruct):
    pass


@AmigaClassDef
class CLI(CLIStruct):
    pass


@AmigaClassDef
class Process(ProcessStruct):
    def new_proc(self):
        self.task.new_task(nt=NodeType.NT_PROCESS)
        self.msg_port.new()
        self.local_vars.new()

    # ----- local variables on pr_LocalVars -----

    def iter_local_vars(self):
        for node in self.local_vars:
            yield LocalVar._bind(self._mem, node.addr)

    def find_var(self, name, vtype):
        for var in self.iter_local_vars():
            if var.node.type.val == vtype and var.name.lower() == name.lower():
                return var
        return None

    def create_var(self, name, vtype):
        var = LocalVar.alloc_var(self._alloc, name, vtype)
        # pr_LocalVars is a MinList holding full Nodes: a Node starts with
        # the MinNode succ/pred pair, so view it as the list's node type
        varlist = self.local_vars
        min_node = varlist.head.get_ref_type()._bind(self._mem, var.addr)
        varlist.add_head(min_node)
        return var

    def set_var_value(self, var, size, src_addr=None, value=None):
        var.set_value(self._alloc, size, src_addr=src_addr, value=value)

    def delete_var(self, var):
        var.free_var(self._alloc)

    def free_local_vars(self):
        for var in list(self.iter_local_vars()):
            self.delete_var(var)

    def copy_local_vars(self, dst):
        """duplicate our variables onto Process dst, as CreateNewProc does for
        a child by default (NP_CopyVars)"""
        for var in self.iter_local_vars():
            new = dst.create_var(var.name, var.node.type.val)
            new.node.pri.val = var.node.pri.val
            new.flags.val = var.flags.val
            size = var.len.val
            if var.value.aptr != 0 and size > 0:
                dst.set_var_value(new, size, src_addr=var.value.aptr)
