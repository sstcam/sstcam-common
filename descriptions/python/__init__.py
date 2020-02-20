from sstcam import _import_module
_m = _import_module(__name__)
__version__ = _m._get_version()
from .ssreadout import SSReadout
from .triggerpatternpacket import TriggerPacket
