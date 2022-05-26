add_library(OPERA_SRC OBJECT
   thread.cpp
   command_line_interface.cpp
   geometry.cpp
   body.cpp
   body_registry.cpp
   topic.cpp
   message.cpp
   state.cpp
   serialisation.cpp
   deserialisation.cpp
   memory.cpp
   mqtt.cpp
   mode.cpp
   trace_sample_range.cpp
   barrier.cpp
   lookahead_job.cpp
   lookahead_job_registry.cpp
   lookahead_job_factory.cpp
   runtime_io.cpp
   runtime.cpp
)

if(COVERAGE)
    target_link_libraries(OPERA_SRC coverage_config)
endif()