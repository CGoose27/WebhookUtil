cmake_minimum_required(VERSION 3.2)
project("Webhook Tester")

# Adding compilation options
add_compile_options("-O3")

add_executable("WebhookUtil" WebhookUtil.c)
target_link_libraries(WebhookUtil libcurl.so)