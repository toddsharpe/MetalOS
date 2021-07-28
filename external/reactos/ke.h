#pragma once

VOID
NTAPI
KiRestoreProcessorControlState(
	IN PKPROCESSOR_STATE ProcessorState
);

VOID
NTAPI
KiSaveProcessorControlState(
	OUT PKPROCESSOR_STATE ProcessorState
);
