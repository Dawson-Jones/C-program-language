cmake_minimum_required(VERSION 2.8)

project(cli)


aux_source_directory(. DIR_SRCS)

add_subdirectory(libmytbf)

add_executable(cli cli.c)

target_link_libraries(cli mytbf)