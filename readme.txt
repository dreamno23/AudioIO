audio IO with OpenSL ES on Android

v0.1.0

初始化输入流/输出流

控制 start，stop，pause等方法
控制流回调
void (*IOStreamCallback)(const audio_buffer *buffer, void *ref);

如果需要处理流数据，直接进行buffer处理

