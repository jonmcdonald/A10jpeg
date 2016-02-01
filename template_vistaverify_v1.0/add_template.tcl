#
# Add custom models template(s) to Vista project
#
# Run this script with "vista_model_builder -c <path_to_this_script>
#
# The directory containing the template models must be specified, as done below
#=================================================================================

# set directory name of this file
set TEMPLATE_VISTAVERIFY_DIR template_vistaverify_v1.0

add_template -f /TLM/Generic_Library/VistaVerify_models/VistaVerify $env(PWD)/$TEMPLATE_VISTAVERIFY_DIR


