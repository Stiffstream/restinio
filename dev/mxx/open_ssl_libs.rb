require 'mxx_ru/cpp'

def find_first_openssl_libs
	openssl_before_11 = [ 'libeay32.lib', 'ssleay32.lib' ]
	openssl_after_11 = [ 'libssl.lib', 'libcrypto.lib' ]


	lib_dirs = ENV[ "LIB" ]
	if lib_dirs
		dir_indx_before_11 = lib_dirs.split( ';' ).find_index{|d|
			File.exist?(File.join(d, openssl_before_11[0])) and
			File.file?(File.join(d, openssl_before_11[0])) and
			File.exist?(File.join(d, openssl_before_11[1])) and
			File.file?(File.join(d, openssl_before_11[1]))
		}
		dir_indx_after_11 = ENV[ "LIB" ].split( ';' ).find_index{|d|
			File.exist?(File.join(d, openssl_after_11[0])) and
			File.file?(File.join(d, openssl_after_11[0])) and
			File.exist?(File.join(d, openssl_after_11[1])) and
			File.file?(File.join(d, openssl_after_11[1]))
		}
	end

	if dir_indx_before_11 and dir_indx_after_11 then
		if dir_indx_after_11 < dir_indx_after_11 then
			openssl_before_11
		else
			openssl_after_11
		end
	elsif dir_indx_before_11 then
		openssl_before_11
	elsif dir_indx_after_11 then
		openssl_after_11
	else
		raise "OpenSSL libs not found; tried [#{openssl_before_11.join(', ')}] and [#{openssl_after_11.join(', ')}]"
	end
end

MxxRu::Cpp::lib_collection_target {

  if 'vc' == toolset.name
	find_first_openssl_libs.each{|l| lib(l)}
  else
    lib 'ssl'
    lib 'crypto'
  end
}

if __FILE__ == $0
	libs = find_first_openssl_libs
	if libs then
		puts "Openssl libs: #{libs.join(', ')}"
	else
		puts "Openssl libs not found"
	end
end