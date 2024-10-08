project(
    'vic-sensor',
    'cpp'
)

source = [
    'vicsensor.cpp'
]

sdbusplus_dep = dependency('sdbusplus')

executable(
    'vic-sensor',
    source,
    dependencies:sdbusplus_dep,
    install: true
)