# Helper functions for searching for external libs.
module RestinioLibFinder

  # Check whether the entry exists and itis a file.
  def self.file_exists?( dir, f )
    fname = File.join( dir, f )

    if File.exist?( fname ) and File.file?( fname )
      fname
    else
      nil
    end
  end

  # Checks if directory contains each file in the list.
  def self.all_files_exist( dir, files )
    # Search for the first file that is not contiained by the directory
    # if all files exists then the search will return nil and
    # `not nil` would result into true - all files exists.
    not files.find{|f| !file_exists?(dir, f ) }
  end

  # Gets the list of a directories tha compiler considers as a search path for libraries.
  # See th output for 'gcc -print-search-dirs' for example.
  def self.get_lib_dirs( toolset )
    libs = []
    IO.popen( "#{toolset.cpp_compiler_name} -print-search-dirs 2>&1",
              :err => [:child, :out] ) do |io|
      io.each_line do |line|
        if /^libraries: =(?<libstr>.*)/ =~ line
          splitter = ':'
          if 'mswin' == toolset.tag( 'target_os' )
            splitter = ';'
          end
          libs += libstr.split( splitter )
        end
      end
    end
    libs
  end

  # Returns the name from environment variable PCRE_PRJ_FILE or
  # name 'local-pcre.rb' if PCRE_PRJ_FILE is not set.
  #
  # Note that this file can be nonexistent. Because of that File.exist?
  # must be used to check presence of this file.
  def self.try_to_get_custom_file( env_tag, file_name )
    custom_local_pcre_prj = ENV[ env_tag ]
    custom_local_pcre_prj = file_name unless custom_local_pcre_prj
    custom_local_pcre_prj
  end

  def self.check_libs_available( toolset, libs )
    if 'mswin' == toolset.tag( 'target_os' )
        if 'vc' == toolset.name
          true
        elsif [ 'gcc', 'clang' ].include? toolset.name
          get_lib_dirs( toolset ).find_index{|d| all_files_exist( d, libs ) }
        end
    else
      get_lib_dirs( toolset ).find_index{|d| all_files_exist( d, libs ) }
    end
  end

end # module RestinioLibFinder
