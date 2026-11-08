#pragma once

class KPipe;
class KRef;

void KePipeUnref(KPipe& pipe, KRef& ref); //Drops one of the pipe's Readers/Writers/Waiters; the pipe is freed when all three are empty
