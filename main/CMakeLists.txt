set(COMPONENT_SRCS "vesync_https.c")
set(COMPONENT_ADD_INCLUDEDIRS ".")

register_component()

# Embed the server root certificate into the final binary
#
# (If this was a component, we would set COMPONENT_EMBED_TXTFILES here.)
set(COMPONENT_EMBED_TXTFILES server_root_cert.pem)

register_component()
