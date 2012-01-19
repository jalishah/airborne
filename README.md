# ARCADE airborne software

this repositories includes:

* __skeleton_project__ (MOBICOM_PROJECT_PATH defines top-level path)
    * MOBICOM_common (MOBICOM common)
        * __scl__: signaling and communication link
        * __svctrl__: service control utility
    * __common__ (project-specific common)
        * __messages__: messages formats exchanged between different subprojects
        * __config__: common configuration files for all subprojects
    * __subproject_a__ (MOBICOM_SUBPROJECT_NAME defines folder)
        * __common__: network-level protobuf message definitions
            * __messages__: scl message definitions
            * __config__: system.yaml, services.yaml, parameters-*.yaml
        * __components__: programs connected through __common__ above
    * __subproject_b__ (MOBICOM_SUBPROJECT_NAME) [same structure as above]
