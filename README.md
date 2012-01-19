# ARCADE airborne software

* __MOBICOM__ (MOBICOM_PATH defines top-level path)
    * __common__ (MOBICOM common)
        * __scl__: signaling and communication link
        * __svctrl__: service control utility
    * __ARCADE__ (MOBICOM_PROJECT_NAME = ARCADE):
        * __common__ (ARCADE common)
            * __messages__: messages formats exchanged between different subprojects
            * __config__: common configuration files for all subprojects
        * __airborne__ (MOBICOM_SUBPROJECT_NAME = airborne)
            * __common__: network-level protobuf message definitions
                * __messages__: scl message definitions
                * __config__: system.yaml, services.yaml, parameters-*.yaml
            * __components__: programs connected through __common__ above
