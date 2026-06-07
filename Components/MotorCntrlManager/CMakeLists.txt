set(SOURCE_FILES
  "${CMAKE_CURRENT_LIST_DIR}/MotorCntrlManager.fpp"
  "${CMAKE_CURRENT_LIST_DIR}/MotorCntrlManager.cpp"
)

set(MOD_DEPS
  Components_RomiTypes
  Components_RomiPorts
)

register_fprime_module()
