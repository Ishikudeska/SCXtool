option(SC_USE_PCH "Precompile the common Qt and standard headers" ON)

set(SC_PCH_HEADERS
    <algorithm> <functional> <memory> <optional> <string> <string_view> <utility> <vector>
    <QByteArray> <QDir> <QFile> <QFileInfo> <QHash> <QList> <QMap> <QObject> <QRegularExpression>
    <QSet> <QString> <QStringList> <QVariant>
)

function(sc_precompile target)
    if(SC_USE_PCH)
        target_precompile_headers(${target} PRIVATE ${SC_PCH_HEADERS} ${ARGN})
    endif()
endfunction()

option(SC_USE_LLD "Link Debug builds with lld when it is available" ON)
if(SC_USE_LLD AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    find_program(SC_LLD ld.lld)
    if(SC_LLD)
        include(CheckLinkerFlag)
        check_linker_flag(CXX "-fuse-ld=lld" SC_HAVE_LLD)
        if(SC_HAVE_LLD)
            add_link_options($<$<CONFIG:Debug>:-fuse-ld=lld>)
        endif()
    endif()
endif()
