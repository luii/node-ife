{

    'conditions': [
        ['OS=="mac" or OS=="freebsd"', {
            'targets': [
                {
                    'target_name': 'IFEBinding',
                    'sources': [
                        'src/wrapper.cc',
                        'src/ife-icmp-support.cc',
                        'src/ife-bpf.cc',
                        'src/arpcache-ctlnet.cc'
                    ],
                    'include_dirs': [
                        "<!(node -e \"require('nan')\")"
                    ],
                }
            ]
        }],
        ['OS=="linux"', {
            'targets': [
                {
                    'target_name': 'IFEBinding',
                    'sources': [
                        'src/wrapper.cc',
                        'src/ife-icmp-support.cc',
                        'src/ife-sockpacket.cc',
                        'src/arpcache-proc.cc'
                    ],
                    'include_dirs': [
                        "<!(node -e \"require('nan')\")"
                    ],
                }
            ]
        }],
        ['OS=="solaris"', {
            'targets': [
                {
                    'target_name': 'IFEBinding',
                    'sources': [
                        'src/wrapper.cc',
                        'src/ife-icmp-support.cc',
                        'src/ife-dlpi.cc',
                        'src/arpcache-dlpi.cc'
                    ],
                    'include_dirs': [
                        "<!(node -e \"require('nan')\")"
                    ],
                }
            ]
        }],
        ['OS=="windows"', {
            'targets': [
                {
                    'target_name': 'IFEBinding',
                    'sources': [
                        'src/wrapper.cc',
                        'src/ife-icmp-support.cc',
                        'src/ife-win32.cc',
                        'src/arpcache-none.cc'
                    ],
                    'include_dirs': [
                        "<!(node -e \"require('nan')\")"
                    ],
                }
            ]
        }],
        ['1==1', {
            'targets': [
                {
                    'target_name': 'IFEStub',
                    'type': 'none',
                    'include_dirs': [
                        "<!(node -e \"require('nan')\")"
                    ],
                }
            ]
        }]
    ]
}
